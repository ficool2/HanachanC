#include "../common.h"
#include "yaz.h"
#include "rkg.h"
#include "../player/input.h"

void rkg_init(rkg_t* rkg)
{
    rkg->frames = NULL;
    rkg->frame_count = 0;
    rkg->name[0] = '\0';
}

void rkg_free(rkg_t* rkg)
{
    free(rkg->frames);
    rkg->frames = NULL;
    rkg->frame_count = 0;
}

int rkg_parse(rkg_t* rkg, bin_t* rkg_buffer)
{
    if (strncmp((char*)rkg_buffer->buffer, "RKGD", 4))
    {
        printf("Error reading RKGD, bad header\n");
        return 0;
    }

    rkg_header_t* header = &rkg->header;
    rkg_time_t* finish_time = &header->finish_time;

    bitstream_t stream;
    bitstream_init(&stream, rkg_buffer->buffer);

                                 bitstream_read_skip  (&stream, 32);
    finish_time->minutes       = bitstream_read_uint8 (&stream, 7);
    finish_time->seconds       = bitstream_read_uint8 (&stream, 7);
    finish_time->milliseconds  = bitstream_read_uint16(&stream, 10);
    header->course_id          = bitstream_read_uint8 (&stream, 6);
                                 bitstream_read_skip  (&stream, 2);
    header->vehicle_id         = bitstream_read_uint8 (&stream, 6);
    header->character_id       = bitstream_read_uint8 (&stream, 6);
    header->year               = bitstream_read_uint8 (&stream, 7);
    header->month              = bitstream_read_uint8 (&stream, 4);
    header->day                = bitstream_read_uint8 (&stream, 5);
    header->controller_id      = bitstream_read_uint8 (&stream, 4);
                                 bitstream_read_skip  (&stream, 4);
    header->compressed_flag    = bitstream_read_uint8 (&stream, 1);
                                 bitstream_read_uint8 (&stream, 2);
    header->ghost_type         = bitstream_read_uint8 (&stream, 7);
    header->drift_type         = bitstream_read_uint8 (&stream, 1);
                                 bitstream_read_skip  (&stream, 1);
    header->input_data_length  = bitstream_read_uint16(&stream, 16);
    header->lap_count          = bitstream_read_uint8 (&stream, 8);
    for (int i = 0; i < 5; i++) 
    {
        rkg_time_t* time       = &header->lap_split_times[i];
        time->minutes          = bitstream_read_uint8 (&stream, 7);
        time->seconds          = bitstream_read_uint8 (&stream, 7);
        time->milliseconds     = bitstream_read_uint16(&stream, 10);
    }
                                 bitstream_read_skip  (&stream, 0x14 * 8);
    header->country_code       = bitstream_read_uint8 (&stream, 8);
    header->state_code         = bitstream_read_uint8 (&stream, 8);
    header->location_code      = bitstream_read_uint16(&stream, 16);
    header->unknown            = bitstream_read_uint32(&stream, 32);
    for (int i = 0; i < 0x4A; i++)
        header->mii_data[i]    = bitstream_read_uint8 (&stream, 8);
    header->crc16              = bitstream_read_uint16(&stream, 16);

    if (stream.pos != 0x88*8)
    {
        printf("Error reading RKGD header (expected: %d, got %d)\n", 0x88*8, stream.pos);
        return 0;
    }

    bin_t input;
    if (header->compressed_flag)
    {
        bin_t compressed_input;
        uint32_t size = bitstream_read_uint32(&stream, 32);
        bin_set(&compressed_input, bitstream_current_data(&stream), size);
        if (size > rkg_max_size)
        {
            printf("Error, compressed input data exceeds size of %d\n", rkg_max_size);
            return 0;
        }

        if (yaz_decompress(&compressed_input, &input) != YAZ_OK)
        {
            printf("Error, decompressing input data failed\n");
            return 0;
        }
    }
    else
    {
        bin_set(&input, bitstream_current_data(&stream), rkg_max_size);
    }

    bitstream_init(&stream, input.buffer);

    rkg_input_header_t* input_header = &rkg->input_header;
    input_header->face_button_count  = bitstream_read_uint16(&stream, 16);
    input_header->direction_count    = bitstream_read_uint16(&stream, 16);
    input_header->trick_count        = bitstream_read_uint16(&stream, 16);
    input_header->unknown            = bitstream_read_uint16(&stream, 16);

    uint16_t* face_buttons           = malloc(input_header->face_button_count * sizeof(uint16_t));
    uint16_t* directions             = malloc(input_header->direction_count * sizeof(uint16_t));
    uint16_t* tricks                 = malloc(input_header->trick_count * sizeof(uint16_t));
    rkg->frame_count                 = 0;

    for (uint32_t i = 0; i < input_header->face_button_count; i++)
    {
        face_buttons[i]              = bitstream_read_uint16(&stream, 16);
        rkg->frame_count             += face_buttons[i] & 0xFF;
    }
    for (uint32_t i = 0; i < input_header->direction_count; i++)
    {
        directions[i]                = bitstream_read_uint16(&stream, 16);
    }
    for (uint32_t i = 0; i < input_header->trick_count; i++)
    {
        tricks[i]                    = bitstream_read_uint16(&stream, 16);
    }

    /* TODO figure out why my calculation here is wrong */
    stream.pos                      -= 4*8;
    rkg->crc32                      = bitstream_read_uint32(&stream, 32);

    rkg->frames                     = calloc(rkg->frame_count, sizeof(*rkg->frames));
    for (uint32_t frame_idx = 0, i = 0; i < input_header->face_button_count; i++)
    {
        uint8_t num   = face_buttons[i] & 0xFF;
        uint8_t value = face_buttons[i] >> 8;
        for (uint8_t j = 0; j < num; j++)
        {
            input_t* frame = &rkg->frames[frame_idx++];
            if (value & 0x01)
                frame->accelerate = true;
            if (value & 0x02)
                frame->brake = true;
            if (value & 0x04)
                frame->use_item = true;
            if (value & 0x08)
                frame->drift = true;
        }
    }
    for (uint32_t frame_idx = 0, i = 0; i < input_header->direction_count; i++)
    {
        uint8_t num   = directions[i] & 0xFF;
        uint8_t value = directions[i] >> 8;
        for (uint8_t j = 0; j < num; j++)
        {
            input_t* frame = &rkg->frames[frame_idx++];
            uint8_t stick_x = value >> 4;
            uint8_t stick_y = value & 0x0F;
            frame->stick_x = ((float)stick_x - 7.0f) / 7.0f;
            frame->stick_y = ((float)stick_y - 7.0f) / 7.0f;
        }
    }
    for (uint32_t frame_idx = 0, i = 0; i < input_header->trick_count; i++)
    {
        uint16_t num  = tricks[i] & 0xFFF;
        uint8_t value = tricks[i] >> 12;
        for (uint16_t j = 0; j < num; j++)
        {
            input_t* frame = &rkg->frames[frame_idx++];
            frame->trick = value;
        }
    }

    free(face_buttons);
    free(directions);
    free(tricks);

    int ret = 1;

    if (header->compressed_flag)
    {
        size_t size_read = bitstream_current_data(&stream) - stream.buffer;
        if (size_read != input.size)
        {
            printf("Error reading input data (expected: %zu, got %zu)\n", input.size, size_read);
            ret = 0;
        }

        bin_free(&input);
    }

    return ret;
}

parser_t rkg_parser =
{
    rkg_init,
    rkg_free,
    rkg_parse,
};