/* ///////////////////////////////////////////////////////////////////////
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2001-2007 Intel Corporation. All Rights Reserved.
//
//  Description:    Parses MPEG-4 headers
//
*/

#include "mp4def.h"


mp4_Status mp4_Parse_VisualObjectSequence(mp4_Info* pInfo)
{
    pInfo->profile_and_level_indication = mp4_GetBits9(pInfo, 8);
    if (pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_0 &&
        pInfo->profile_and_level_indication != MP4_CORE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_CORE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_MAIN_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_MAIN_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_MAIN_PROFILE_LEVEL_4 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_REAL_TIME_SIMPLE_PROFILE_LEVEL_4 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_CODING_EFFICIENCY_PROFILE_LEVEL_4 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_CORE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_CORE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_0 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_4 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_5 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_3B) {
        mp4_Error("Warning: Unsupported profile_and_level_indication");
        if (pInfo->strictSyntaxCheck)
            return MP4_STATUS_NOTSUPPORT;
    }
    if (pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_SIMPLE_PROFILE_LEVEL_0 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_0 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_1 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_2 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_3 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_4 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_5 &&
        pInfo->profile_and_level_indication != MP4_ADVANCED_SIMPLE_PROFILE_LEVEL_3B) {
        mp4_Error("Warning: Some features of present profile_and_level_indication may be not supported");
    }
    return MP4_STATUS_OK;
}


mp4_Status mp4_Parse_VisualObject(mp4_Info* pInfo)
{
    mp4_VisualObject  *VO = &pInfo->VisualObject;

    VO->is_identifier = mp4_GetBit(pInfo);
    if (VO->is_identifier) {
        VO->verid = mp4_GetBits9(pInfo, 4);
        if ((VO->verid != 1) && (VO->verid != 2) && (VO->verid != 4) && (VO->verid != 5)) {
            VO->verid = 1;
            mp4_Error("Warning: Unsupported visual_object_verid");
            //f if (pInfo->strictSyntaxCheck)
            //f     return MP4_STATUS_NOTSUPPORT;
        }
        VO->priority = mp4_GetBits9(pInfo, 3);
        if (VO->priority == 0) {
            mp4_Error("Warning: Wrong value of visual_object_priority");
            //f if (pInfo->strictSyntaxCheck)
            //f     return MP4_STATUS_PARSE_ERROR;
        }
    } else
        VO->verid = 1;
    VO->type = mp4_GetBits9(pInfo, 4);
    if (VO->type != MP4_VISUAL_OBJECT_TYPE_VIDEO) {
        mp4_Error("Error: Unsupported object: visual_object_type != video ID");
        return MP4_STATUS_NOTSUPPORT;
    }
    VO->VideoSignalType.video_format = MP4_VIDEO_FORMAT_UNSPECIFIED;
    VO->VideoSignalType.video_range = 0;
    VO->VideoSignalType.colour_primaries = MP4_VIDEO_COLORS_ITU_R_BT_709;
    VO->VideoSignalType.transfer_characteristics = MP4_VIDEO_COLORS_ITU_R_BT_709;
    VO->VideoSignalType.matrix_coefficients = MP4_VIDEO_COLORS_ITU_R_BT_709;
    if (VO->type == MP4_VISUAL_OBJECT_TYPE_VIDEO || VO->type == MP4_VISUAL_OBJECT_TYPE_TEXTURE) {
        VO->VideoSignalType.is_video_signal_type = mp4_GetBit(pInfo);
        if (VO->VideoSignalType.is_video_signal_type) {
            VO->VideoSignalType.video_format = mp4_GetBits9(pInfo, 3);
            if (VO->VideoSignalType.video_format > MP4_VIDEO_FORMAT_UNSPECIFIED) {
                mp4_Error("Warning: Wrong value of video_format");
                //f if (pInfo->strictSyntaxCheck)
                //f     return MP4_STATUS_PARSE_ERROR;
            }
            VO->VideoSignalType.video_range = mp4_GetBit(pInfo);
            VO->VideoSignalType.is_colour_description = mp4_GetBit(pInfo);
            if (VO->VideoSignalType.is_colour_description) {
                VO->VideoSignalType.colour_primaries = mp4_GetBits9(pInfo, 8);
                if (VO->VideoSignalType.colour_primaries == MP4_VIDEO_COLORS_FORBIDDEN) {
                    mp4_Error("Error: Wrong value of colour_primaries");
                    if (pInfo->strictSyntaxCheck)
                        return MP4_STATUS_PARSE_ERROR;
                }
                if (VO->VideoSignalType.colour_primaries > MP4_VIDEO_COLORS_GENERIC_FILM) {
                    mp4_Error("Warning: Wrong value of colour_primaries");
                    //f if (pInfo->strictSyntaxCheck)
                    //f     return MP4_STATUS_PARSE_ERROR;
                }
                VO->VideoSignalType.transfer_characteristics = mp4_GetBits9(pInfo, 8);
                if (VO->VideoSignalType.transfer_characteristics == 0) {
                    mp4_Error("Error: Wrong value of transfer_characteristics");
                    if (pInfo->strictSyntaxCheck)
                        return MP4_STATUS_PARSE_ERROR;
                }
                if (VO->VideoSignalType.transfer_characteristics > 10) {
                    mp4_Error("Warning: Wrong value of transfer_characteristics");
                    //f if (pInfo->strictSyntaxCheck)
                    //f     return MP4_STATUS_PARSE_ERROR;
                }
                VO->VideoSignalType.matrix_coefficients = mp4_GetBits9(pInfo, 8);
                if (VO->VideoSignalType.matrix_coefficients == 0) {
                    mp4_Error("Error: Wrong value of matrix_coefficients");
                    if (pInfo->strictSyntaxCheck)
                        return MP4_STATUS_PARSE_ERROR;
                }
                if (VO->VideoSignalType.matrix_coefficients > 8) {
                    mp4_Error("Warning: Wrong value of matrix_coefficients");
                    //f if (pInfo->strictSyntaxCheck)
                    //f     return MP4_STATUS_PARSE_ERROR;
                }
            }
        }
    }
    return MP4_STATUS_OK;
}


static mp4_Status mp4_Parse_QuantMatrix(mp4_Info* pInfo, uint8_t pQM[64])
{
    uint32_t  code;
    int32_t  i;

    for (i = 0; i < 64; i ++) {
        code = mp4_GetBits9(pInfo, 8);
        if (code == 0) break;
            pQM[mp4_ClassicalZigzag[i]] = (uint8_t)code;
    }
    if (i > 0) {
        code = pQM[mp4_ClassicalZigzag[i-1]];
        for (; i < 64; i ++)
            pQM[mp4_ClassicalZigzag[i]] = (uint8_t)code;
    } else {
        for (i = 1; i < 64; i ++)
            pQM[mp4_ClassicalZigzag[i]] = (uint8_t)mp4_ClassicalZigzag[0];
    }
    return MP4_STATUS_OK;
}


mp4_Status mp4_Parse_VideoObject(mp4_Info* pInfo)
{
    uint32_t  code;
    int32_t  i;
    mp4_VisualObject  *VO = &pInfo->VisualObject;
    mp4_VideoObject  *VOL = &pInfo->VisualObject.VideoObject;

    code = mp4_ShowBits(pInfo, 32);
    // check short_video_start_marker
    if ((code & (~0x3FF)) == 0x8000) {
        VOL->short_video_header = 1;
        VOL->quant_precision = 5;
        VOL->shape = MP4_SHAPE_TYPE_RECTANGULAR;
        VOL->obmc_disable = 1;
        VOL->quant_type = 0;
        VOL->resync_marker_disable = 1;
        VOL->data_partitioned = 0;
        VOL->reversible_vlc = 0;
        VOL->VideoObjectPlane.rounding_type = 0;
        VOL->VideoObjectPlane.fcode_forward = 1;
        VOL->VideoObjectPlane.coded = 1;
        VOL->interlaced = 0;
        VOL->complexity_estimation_disable = 1;
        VOL->scalability = 0;
        VOL->not_8_bit = 0;
        VOL->bits_per_pixel = 8;
        VO->VideoSignalType.colour_primaries = 1;
        VO->VideoSignalType.transfer_characteristics = 1;
        VO->VideoSignalType.matrix_coefficients = 6;
        VO->VideoObject.VideoObjectPlaneH263.source_format = (pInfo->bufptr[4] >> 2) & 0x7;
        i = VO->VideoObject.VideoObjectPlaneH263.source_format - 1;
        if (i < 0 || i > 4) {
            mp4_Error("Error: Bad value for VideoPlaneWithShortHeader.source_format");
            return MP4_STATUS_PARSE_ERROR;
        }
        VOL->width = mp4_H263_width[i];
        VOL->height = mp4_H263_height[i];
        VOL->VideoObjectPlaneH263.num_gobs_in_vop = mp4_H263_gobvop[i];
        VOL->VideoObjectPlaneH263.num_macroblocks_in_gob = mp4_H263_mbgob[i];
        VOL->VideoObjectPlaneH263.num_rows_in_gob = mp4_H263_rowgob[i];
        VOL->vop_time_increment_resolution = 30000;
        // VOL->VideoObjectPlane.time_increment = -1001;
        VOL->VOLControlParameters.low_delay = 1;
        return MP4_STATUS_OK;
    }
    if (code < 256 + MP4_VIDEO_OBJECT_LAYER_MIN_SC || code > 256 + MP4_VIDEO_OBJECT_LAYER_MAX_SC) {
        mp4_Error("Error: Bad start code for VideoObjectLayer");
        return MP4_STATUS_PARSE_ERROR;
    }
    mp4_FlushBits(pInfo, 32);
    // video_object_start_code is founded
    VOL->id = code & 15;
    VOL->short_video_header = 0;
    VOL->random_accessible_vol = mp4_GetBit(pInfo);
    VOL->type_indication = mp4_GetBits9(pInfo, 8);
    if (VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_SIMPLE &&
        VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_CORE &&
        VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_MAIN &&
        VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_ADVANCED_REAL_TIME_SIMPLE &&
        VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_ADVANCED_CODING_EFFICIENCY &&
        VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_ADVANCED_SIMPLE) {
        mp4_Error("Warning: Unsupported video_object_type_indication, some features may be not supported");
        //f if (pInfo->strictSyntaxCheck)
        //f     return MP4_STATUS_NOTSUPPORT;
    }
    VOL->is_identifier = mp4_GetBit(pInfo);
    if (VOL->is_identifier) {
        VOL->verid = mp4_GetBits9(pInfo, 4);
        if ((VOL->verid != 1) && (VOL->verid != 2) && (VOL->verid != 4) && (VOL->verid != 5)) {
            mp4_Error("Warning: Unsupported video_object_layer_verid");
            VOL->verid = VO->verid;
            //f if (pInfo->strictSyntaxCheck)
            //f     return MP4_STATUS_NOTSUPPORT;
        }
        VOL->priority = mp4_GetBits9(pInfo, 3);
        if (VOL->priority == 0) {
            mp4_Error("Warning: Wrong value of video_object_layer_priority");
            //f if (pInfo->strictSyntaxCheck)
            //f     return MP4_STATUS_PARSE_ERROR;
        }
    } else
        VOL->verid = VO->verid;
    VOL->aspect_ratio_info = mp4_GetBits9(pInfo, 4);
    if (VOL->aspect_ratio_info == MP4_ASPECT_RATIO_FORBIDDEN) {
        mp4_Error("Error: Wrong value of aspect_ratio_info");
        if (pInfo->strictSyntaxCheck)
            return MP4_STATUS_PARSE_ERROR;
    }
    if (VOL->aspect_ratio_info > MP4_ASPECT_RATIO_40_33 && VOL->aspect_ratio_info < MP4_ASPECT_RATIO_EXTPAR) {
        mp4_Error("Warning: Wrong value of aspect_ratio_info");
        //f if (pInfo->strictSyntaxCheck)
        //f     return MP4_STATUS_PARSE_ERROR;
    }
    if (VOL->aspect_ratio_info == MP4_ASPECT_RATIO_EXTPAR) {
        VOL->aspect_ratio_info_par_width = mp4_GetBits9(pInfo, 8);
        if (VOL->aspect_ratio_info_par_width == 0) {
            mp4_Error("Error: Wrong value of par_width");
            if (pInfo->strictSyntaxCheck)
                return MP4_STATUS_PARSE_ERROR;
        }
        VOL->aspect_ratio_info_par_height = mp4_GetBits9(pInfo, 8);
        if (VOL->aspect_ratio_info_par_height == 0) {
            mp4_Error("Error: Wrong value of par_height");
            if (pInfo->strictSyntaxCheck)
                return MP4_STATUS_PARSE_ERROR;
        }
    }
    VOL->is_vol_control_parameters = mp4_GetBit(pInfo);
    if (VOL->type_indication == MP4_VIDEO_OBJECT_TYPE_SIMPLE || VOL->type_indication != MP4_VIDEO_OBJECT_TYPE_ADVANCED_REAL_TIME_SIMPLE)
        VOL->VOLControlParameters.low_delay = 1;
    if (VOL->is_vol_control_parameters) {
        VOL->VOLControlParameters.chroma_format = mp4_GetBits9(pInfo, 2);
        if (VOL->VOLControlParameters.chroma_format != MP4_CHROMA_FORMAT_420) {
            mp4_Error("Error: chroma_format != 4:2:0");
            return MP4_STATUS_PARSE_ERROR;
        }
        VOL->VOLControlParameters.low_delay = mp4_GetBit(pInfo);
        VOL->VOLControlParameters.vbv_parameters = mp4_GetBit(pInfo);
        if (VOL->VOLControlParameters.vbv_parameters) {
            VOL->VOLControlParameters.bit_rate = mp4_GetBits(pInfo, 15) << 15;
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOL->VOLControlParameters.bit_rate += mp4_GetBits(pInfo, 15);
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            if (VOL->VOLControlParameters.bit_rate == 0) {
                mp4_Error("Error: vbv_parameters bit_rate == 0");
                return MP4_STATUS_PARSE_ERROR;
            }
            VOL->VOLControlParameters.vbv_buffer_size = mp4_GetBits(pInfo, 15) << 3;
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOL->VOLControlParameters.vbv_buffer_size += mp4_GetBits9(pInfo, 3);
            if (VOL->VOLControlParameters.vbv_buffer_size == 0) {
                mp4_Error("Error: vbv_parameters vbv_buffer_size == 0");
                return MP4_STATUS_PARSE_ERROR;
            }
            VOL->VOLControlParameters.vbv_occupancy = mp4_GetBits(pInfo, 11) << 15;
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOL->VOLControlParameters.vbv_occupancy += mp4_GetBits(pInfo, 15);
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
        }
    }
    VOL->shape = mp4_GetBits9(pInfo, 2);
    if (VOL->shape != MP4_SHAPE_TYPE_RECTANGULAR) {
        mp4_Error("Error: video_object_layer_shape != rectangular (not supported)");
        return MP4_STATUS_PARSE_ERROR;
    }
    if (VOL->verid != 1 && VOL->shape == MP4_SHAPE_TYPE_GRAYSCALE) {
        VOL->shape_extension = mp4_GetBits9(pInfo, 4);
        if (VOL->shape_extension >= MP4_SHAPE_EXT_NUM) {
            mp4_Error("Error: wrong value of video_object_layer_shape_extension");
            return MP4_STATUS_PARSE_ERROR;
        }
    } else
        VOL->shape_extension = MP4_SHAPE_EXT_NUM;
    if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
    VOL->vop_time_increment_resolution = mp4_GetBits(pInfo, 16);
    if (VOL->vop_time_increment_resolution == 0) {
        mp4_Error("Error: wrong value of vop_time_increment_resolution");
        return MP4_STATUS_PARSE_ERROR;
    }
    if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
    // define number bits in vop_time_increment_resolution
    code = VOL->vop_time_increment_resolution - 1;
    i = 0;
    do {
        code >>= 1;
        i ++;
    } while (code);
    VOL->vop_time_increment_resolution_bits = i;
    VOL->fixed_vop_rate = mp4_GetBit(pInfo);
    if (VOL->fixed_vop_rate) {
        VOL->fixed_vop_time_increment = mp4_GetBits(pInfo, VOL->vop_time_increment_resolution_bits);
        if (VOL->fixed_vop_time_increment == 0) {
            mp4_Error("Error: wrong value of fixed_vop_time_increment");
            return MP4_STATUS_PARSE_ERROR;
        }
    }
    if (VOL->shape != MP4_SHAPE_TYPE_BINARYONLY) {
        if (VOL->shape == MP4_SHAPE_TYPE_RECTANGULAR) {
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOL->width = mp4_GetBits(pInfo, 13);
            if (VOL->width == 0) {
                mp4_Error("Error: wrong value of video_object_layer_width");
                return MP4_STATUS_PARSE_ERROR;
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOL->height = mp4_GetBits(pInfo, 13);
            if (VOL->height == 0) {
                mp4_Error("Error: wrong value of video_object_layer_height");
                return MP4_STATUS_PARSE_ERROR;
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
        }
        VOL->interlaced = mp4_GetBit(pInfo);
        VOL->obmc_disable = mp4_GetBit(pInfo);
        VOL->sprite_enable = mp4_GetBits9(pInfo, VOL->verid != 1 ? 2 : 1);
        if (VOL->sprite_enable == MP4_SPRITE_STATIC || VOL->sprite_enable == MP4_SPRITE_GMC) {
            if (VOL->sprite_enable == MP4_SPRITE_STATIC) {
                VOL->sprite_width = mp4_GetBits(pInfo, 13);
                if (VOL->sprite_width == 0) {
                    mp4_Error("Error: wrong value of sprite_width");
                    return MP4_STATUS_PARSE_ERROR;
                }
                if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
                VOL->sprite_height = mp4_GetBits(pInfo, 13);
                if (VOL->sprite_height == 0) {
                    mp4_Error("Error: wrong value of sprite_height");
                    return MP4_STATUS_PARSE_ERROR;
                }
                if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
                VOL->sprite_left_coordinate = mp4_GetBits(pInfo, 13);
                VOL->sprite_left_coordinate <<= (32 - 13);
                VOL->sprite_left_coordinate >>= (32 - 13);
                if (VOL->sprite_left_coordinate & 1) {
                    mp4_Error("Error: sprite_left_coordinate must be divisible by 2");
                    return MP4_STATUS_PARSE_ERROR;
                }
                if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
                VOL->sprite_top_coordinate = mp4_GetBits(pInfo, 13);
                VOL->sprite_top_coordinate <<= (32 - 13);
                VOL->sprite_top_coordinate >>= (32 - 13);
                if (VOL->sprite_top_coordinate & 1) {
                    mp4_Error("Error: sprite_top_coordinate must be divisible by 2");
                    return MP4_STATUS_PARSE_ERROR;
                }
                if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            }
            VOL->sprite_warping_points = mp4_GetBits9(pInfo, 6);
            if (VOL->sprite_warping_points > 4 || (VOL->sprite_warping_points == 4 && VOL->sprite_enable == MP4_SPRITE_GMC)) {
                mp4_Error("Error: bad no_of_sprite_warping_points");
                return MP4_STATUS_PARSE_ERROR;
            }
            VOL->sprite_warping_accuracy = mp4_GetBits9(pInfo, 2);
            VOL->sprite_brightness_change = mp4_GetBit(pInfo);
            if (VOL->sprite_enable == MP4_SPRITE_GMC) {
                if (VOL->sprite_brightness_change) {
                    mp4_Error("Error: sprite_brightness_change should be 0 for GMC sprites");
                    return MP4_STATUS_PARSE_ERROR;
                }
            }
            if (VOL->sprite_enable != MP4_SPRITE_GMC) {
                VOL->low_latency_sprite_enable = mp4_GetBit(pInfo);
                if (VOL->low_latency_sprite_enable) {
                    mp4_Error("Error: low_latency_sprite is not supported");
                    return MP4_STATUS_PARSE_ERROR;
                }
            }
        }
        if (VOL->verid != 1 && VOL->shape != MP4_SHAPE_TYPE_RECTANGULAR) {
            VOL->sadct_disable = mp4_GetBit(pInfo);
            if (!VOL->sadct_disable) {
                mp4_Error("Error: Shape Adaptive DCT is not supported");
                return MP4_STATUS_PARSE_ERROR;
            }
        }
        VOL->not_8_bit = mp4_GetBit(pInfo);
        if (VOL->not_8_bit) {
            mp4_Error("Error: only 8-bits data is supported");
            return MP4_STATUS_PARSE_ERROR;
        }
        if (VOL->not_8_bit) {
            VOL->quant_precision = mp4_GetBits9(pInfo, 4);
            if (VOL->quant_precision < 3 || VOL->quant_precision > 9) {
                mp4_Error("Error: quant_precision must be in range [3; 9]");
                return MP4_STATUS_PARSE_ERROR;
            }
            VOL->bits_per_pixel = mp4_GetBits9(pInfo, 4);
            if (VOL->bits_per_pixel < 4 || VOL->bits_per_pixel > 12) {
                mp4_Error("Error: bits_per_pixel must be in range [4; 12]");
                return MP4_STATUS_PARSE_ERROR;
            }
        } else {
            VOL->quant_precision = 5;
            VOL->bits_per_pixel = 8;
        }
        if (VOL->shape == MP4_SHAPE_TYPE_GRAYSCALE) {
            VOL->no_gray_quant_update = mp4_GetBit(pInfo);
            VOL->composition_method = mp4_GetBit(pInfo);
            VOL->linear_composition = mp4_GetBit(pInfo);
        }
        VOL->quant_type = mp4_GetBit(pInfo);
        if (VOL->quant_type) {
            VOL->load_intra_quant_mat = mp4_GetBit(pInfo);
            if (VOL->load_intra_quant_mat) {
                if (mp4_Parse_QuantMatrix(pInfo, VOL->intra_quant_mat) != MP4_STATUS_OK)
                    return MP4_STATUS_PARSE_ERROR;
            } else
                ippsCopy_8u(mp4_DefaultIntraQuantMatrix, VOL->intra_quant_mat, 64);
            VOL->load_nonintra_quant_mat = mp4_GetBit(pInfo);
            if (VOL->load_nonintra_quant_mat) {
                if (mp4_Parse_QuantMatrix(pInfo, VOL->nonintra_quant_mat) != MP4_STATUS_OK)
                    return MP4_STATUS_PARSE_ERROR;
            } else
                ippsCopy_8u(mp4_DefaultNonIntraQuantMatrix, VOL->nonintra_quant_mat, 64);
            if (VOL->shape == MP4_SHAPE_TYPE_GRAYSCALE) {
                int32_t   ac, i;

                ac = mp4_aux_comp_count[VOL->shape_extension];
                for (i = 0; i < ac; i ++) {
                    VOL->load_intra_quant_mat_grayscale[i] = mp4_GetBit(pInfo);
                    if (VOL->load_intra_quant_mat_grayscale[i]) {
                        if (mp4_Parse_QuantMatrix(pInfo, VOL->intra_quant_mat_grayscale[i]) != MP4_STATUS_OK)
                            return MP4_STATUS_PARSE_ERROR;
                    } else
                        ippsCopy_8u(mp4_DefaultIntraQuantMatrix, VOL->intra_quant_mat_grayscale[i], 64);
                    VOL->load_nonintra_quant_mat_grayscale[i] = mp4_GetBit(pInfo);
                    if (VOL->load_nonintra_quant_mat_grayscale[i]) {
                        if (mp4_Parse_QuantMatrix(pInfo, VOL->nonintra_quant_mat_grayscale[i]) != MP4_STATUS_OK)
                            return MP4_STATUS_PARSE_ERROR;
                    } else
                        ippsCopy_8u(mp4_DefaultNonIntraQuantMatrix, VOL->nonintra_quant_mat_grayscale[i], 64);
                }
            }
        }
        if (VOL->verid != 1)
            VOL->quarter_sample = mp4_GetBit(pInfo);
        VOL->complexity_estimation_disable = mp4_GetBit(pInfo);
        if (!VOL->complexity_estimation_disable) {
            VOL->ComplexityEstimation.estimation_method = mp4_GetBits9(pInfo, 2);
            if (VOL->ComplexityEstimation.estimation_method <= 1) {
                VOL->ComplexityEstimation.shape_complexity_estimation_disable = mp4_GetBit(pInfo);
                if (!VOL->ComplexityEstimation.shape_complexity_estimation_disable) {
                    VOL->ComplexityEstimation.opaque =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.transparent =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.intra_cae =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.inter_cae =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.no_update =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.upsampling =  mp4_GetBit(pInfo);
                }
                VOL->ComplexityEstimation.texture_complexity_estimation_set_1_disable =  mp4_GetBit(pInfo);
                if (!VOL->ComplexityEstimation.texture_complexity_estimation_set_1_disable) {
                    VOL->ComplexityEstimation.intra_blocks =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.inter_blocks =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.inter4v_blocks =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.not_coded_blocks =  mp4_GetBit(pInfo);
                }
                if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
                VOL->ComplexityEstimation.texture_complexity_estimation_set_2_disable =  mp4_GetBit(pInfo);
                if (!VOL->ComplexityEstimation.texture_complexity_estimation_set_2_disable) {
                    VOL->ComplexityEstimation.dct_coefs =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.dct_lines =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.vlc_symbols =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.vlc_bits =  mp4_GetBit(pInfo);
                }
                VOL->ComplexityEstimation.motion_compensation_complexity_disable =  mp4_GetBit(pInfo);
                if (!VOL->ComplexityEstimation.motion_compensation_complexity_disable) {
                    VOL->ComplexityEstimation.apm =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.npm =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.interpolate_mc_q =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.forw_back_mc_q =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.halfpel2 =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.halfpel4 =  mp4_GetBit(pInfo);
                }
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            if (VOL->ComplexityEstimation.estimation_method == 1) {
                // verid != 1
                VOL->ComplexityEstimation.version2_complexity_estimation_disable =  mp4_GetBit(pInfo);
                if (!VOL->ComplexityEstimation.version2_complexity_estimation_disable) {
                    VOL->ComplexityEstimation.sadct =  mp4_GetBit(pInfo);
                    VOL->ComplexityEstimation.quarterpel =  mp4_GetBit(pInfo);
                }
            }
        }
        VOL->resync_marker_disable = mp4_GetBit(pInfo);
        VOL->data_partitioned = mp4_GetBit(pInfo);
//f GrayScale Shapes does not support data_part
        if (VOL->data_partitioned)
            VOL->reversible_vlc = mp4_GetBit(pInfo);
        if (VOL->verid != 1) {
            VOL->newpred_enable = mp4_GetBit(pInfo);
            if (VOL->newpred_enable) {
                VOL->requested_upstream_message_type = mp4_GetBits9(pInfo, 2);
                VOL->newpred_segment_type = mp4_GetBit(pInfo);
                mp4_Error("Warning: NEWPRED mode is not fully supported");
//f                return MP4_STATUS_PARSE_ERROR;
            }
            VOL->reduced_resolution_vop_enable = mp4_GetBit(pInfo);
            if (VOL->reduced_resolution_vop_enable) {
                mp4_Error("Error: Reduced Resolution VOP is not supported");
                return MP4_STATUS_PARSE_ERROR;
            }
        }
        VOL->scalability = mp4_GetBit(pInfo);
        if (VOL->scalability) {
            mp4_Error("Error: VOL scalability is not supported");
            return MP4_STATUS_PARSE_ERROR;
        }
    } else {
        if (VOL->verid != 1) {
            VOL->scalability = mp4_GetBit(pInfo);
            if (VOL->scalability) {
                mp4_Error("Error: VOL scalability is not supported");
                return MP4_STATUS_PARSE_ERROR;
            }
        }
        VOL->resync_marker_disable = mp4_GetBit(pInfo);
    }
    VOL->VideoObjectPlane.sprite_transmit_mode = MP4_SPRITE_TRANSMIT_MODE_PIECE;
#if 0
    {
        uint8_t *sbp = pInfo->bufptr;
        int32_t   v, b, i;
        if (mp4_SeekStartCodeValue(pInfo, MP4_USER_DATA_SC)) {
            if (pInfo->bufptr[0] == 'D' && pInfo->bufptr[1] == 'i' && pInfo->bufptr[2] == 'v' && pInfo->bufptr[3] == 'X') {
                pInfo->ftype = 2;
                v = (pInfo->bufptr[4] - '0') * 100 + (pInfo->bufptr[5] - '0') * 10 + (pInfo->bufptr[6] - '0');
                if (v < 503)
                    pInfo->ftype_f = 1;
                else if (v == 503) {
                    i = 8;
                    b = 0;
                    while (pInfo->bufptr[i] != 0) {
                        if (pInfo->bufptr[i] >= '0' && pInfo->bufptr[i] <= '9')
                            b = b * 10 + (pInfo->bufptr[i] - '0');
                        i ++;
                    }
                    pInfo->ftype_f = (b < 959) ? 1 : 0;
                } else
                    pInfo->ftype_f = 0;
            }
        }
        pInfo->bufptr = sbp;
    }
#else
    {
        uint8_t *sbp = pInfo->bufptr;
        if (mp4_SeekStartCodeValue(pInfo, MP4_USER_DATA_SC)) {
            if (pInfo->bufptr[0] == 'D' && pInfo->bufptr[1] == 'i' && pInfo->bufptr[2] == 'v' && pInfo->bufptr[3] == 'X') {
                pInfo->ftype = 2;
                pInfo->ftype_f = 1;
            }
        }
        pInfo->bufptr = sbp;
    }
#endif
    return MP4_STATUS_OK;
}


/*static */mp4_Status mp4_Sprite_Trajectory(mp4_Info* pInfo) {
    int32_t  i, dmv_code, dmv_length, fb;
    uint32_t  code;

    for (i = 0; i < pInfo->VisualObject.VideoObject.sprite_warping_points; i ++) {
        code = mp4_ShowBits9(pInfo, 3);
        if (code == 7) {
            mp4_FlushBits(pInfo, 3);
            code = mp4_ShowBits9(pInfo, 9);
            fb = 1;
            while (code & 256) {
                code <<= 1;
                fb ++;
            }
            if (fb > 9) {
                mp4_Error("Error when decode sprite_trajectory");
                return MP4_STATUS_PARSE_ERROR;
            }
            dmv_length = fb + 5;
        } else {
            fb = (code <= 1) ? 2 : 3;
            dmv_length = code - 1;
        }
        mp4_FlushBits(pInfo, fb);
        if (dmv_length <= 0)
            dmv_code = 0;
        else {
            dmv_code = mp4_GetBits(pInfo, dmv_length);
            if ((dmv_code & (1 << (dmv_length - 1))) == 0)
                dmv_code -= (1 << dmv_length) - 1;
        }
        if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
        pInfo->VisualObject.VideoObject.VideoObjectPlane.warping_mv_code_du[i] = dmv_code;
        code = mp4_ShowBits9(pInfo, 3);
        if (code == 7) {
            mp4_FlushBits(pInfo, 3);
            code = mp4_ShowBits9(pInfo, 9);
            fb = 1;
            while (code & 256) {
                code <<= 1;
                fb ++;
            }
            if (fb > 9) {
                mp4_Error("Error when decode sprite_trajectory");
                return MP4_STATUS_PARSE_ERROR;
            }
            dmv_length = fb + 5;
        } else {
            fb = (code <= 1) ? 2 : 3;
            dmv_length = code - 1;
        }
        mp4_FlushBits(pInfo, fb);
        if (dmv_length <= 0)
            dmv_code = 0;
        else {
            dmv_code = mp4_GetBits(pInfo, dmv_length);
            if ((dmv_code & (1 << (dmv_length - 1))) == 0)
                dmv_code -= (1 << dmv_length) - 1;
        }
        if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
        pInfo->VisualObject.VideoObject.VideoObjectPlane.warping_mv_code_dv[i] = dmv_code;
    }
    return MP4_STATUS_OK;
}


mp4_Status mp4_Parse_GroupOfVideoObjectPlane(mp4_Info* pInfo)
{
    uint32_t  hour, min, sec;

    hour = mp4_GetBits9(pInfo, 5);
    min = mp4_GetBits9(pInfo, 6);
    if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
    sec = mp4_GetBits9(pInfo, 6);
    pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.time_code = sec + min * 60 + hour * 3600;
    pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.time_code *= pInfo->VisualObject.VideoObject.vop_time_increment_resolution;
    pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.closed_gov = mp4_GetBit(pInfo);
    pInfo->VisualObject.VideoObject.GroupOfVideoObjectPlane.broken_link = mp4_GetBit(pInfo);
    return MP4_STATUS_OK;
}


mp4_Status mp4_Parse_VideoObjectPlane(mp4_Info* pInfo)
{
    uint32_t  code;
    mp4_VideoObject          *VOL = &pInfo->VisualObject.VideoObject;
    mp4_VideoObjectPlane     *VOP = &pInfo->VisualObject.VideoObject.VideoObjectPlane;
    mp4_VideoObjectPlaneH263 *VOPSH = &pInfo->VisualObject.VideoObject.VideoObjectPlaneH263;

    if (VOL->short_video_header) {
        code = mp4_GetBits9(pInfo, 6); // read rest bits of short_video_start_marker
        VOPSH->temporal_reference = mp4_GetBits9(pInfo, 8);
        if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
        code = mp4_GetBit(pInfo); // zero_bit
        VOPSH->split_screen_indicator = mp4_GetBit(pInfo);
        VOPSH->document_camera_indicator = mp4_GetBit(pInfo);
        VOPSH->full_picture_freeze_release = mp4_GetBit(pInfo);
        VOPSH->source_format = mp4_GetBits9(pInfo, 3);
        if (VOPSH->source_format == 0 || VOPSH->source_format > 5) {
            mp4_Error("Error: Bad value for VideoPlaneWithShortHeader.source_format");
            return MP4_STATUS_PARSE_ERROR;
        }
        VOPSH->picture_coding_type = mp4_GetBit(pInfo);
        VOP->coding_type = VOPSH->picture_coding_type;
        code = mp4_GetBits9(pInfo, 4); // four_reserved_zero_bits
        VOPSH->vop_quant = mp4_GetBits9(pInfo, 5);
        code = mp4_GetBit(pInfo); // zero_bit
        for (;;) {
            code = mp4_GetBit(pInfo); // pei
            if (!code)
                break;
            code = mp4_GetBits9(pInfo, 8); // psupp
        }
        return MP4_STATUS_OK;
    }
    VOP->coding_type = mp4_GetBits9(pInfo, 2);
    if (((VOP->coding_type != MP4_VOP_TYPE_I) && (pInfo->noPVOPs)) || ((VOP->coding_type == MP4_VOP_TYPE_B) && (pInfo->noBVOPs))) {
        mp4_Error("Error: Bad vop_coding_type");
        return MP4_STATUS_PARSE_ERROR;
    }
    VOP->modulo_time_base = 0;
    do {
        code = mp4_GetBit(pInfo);
        VOP->modulo_time_base += code;
    } while (code);
    if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
    if (VOL->vop_time_increment_resolution_bits != 0 ) {
        VOP->time_increment = mp4_GetBits(pInfo, VOL->vop_time_increment_resolution_bits);
    }
    if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
    VOP->coded = mp4_GetBit(pInfo);
    if (!VOP->coded)
        return MP4_STATUS_OK;
    if (VOL->newpred_enable) {
        int vop_id_length = VOL->vop_time_increment_resolution_bits + 3;
        if (vop_id_length > 15)
            vop_id_length = 15;
        VOP->vop_id = mp4_GetBits(pInfo, vop_id_length);
        VOP->vop_id_for_prediction_indication = mp4_GetBit(pInfo);
        if (VOP->vop_id_for_prediction_indication)
            VOP->vop_id_for_prediction = mp4_GetBits(pInfo, vop_id_length);
        if (!mp4_GetMarkerBit(pInfo))
            return MP4_STATUS_PARSE_ERROR;
    }
    if (VOL->shape != MP4_SHAPE_TYPE_BINARYONLY && (VOP->coding_type == MP4_VOP_TYPE_P ||
        (VOP->coding_type == MP4_VOP_TYPE_S && VOL->sprite_enable == MP4_SPRITE_GMC)))
        VOP->rounding_type = mp4_GetBit(pInfo);
    if (VOL->reduced_resolution_vop_enable && VOL->shape == MP4_SHAPE_TYPE_RECTANGULAR &&
        (VOP->coding_type == MP4_VOP_TYPE_I || VOP->coding_type == MP4_VOP_TYPE_P)) {
        VOP->reduced_resolution = mp4_GetBit(pInfo);
        if (VOP->reduced_resolution) {
            mp4_Error("Error: Reduced Resolution VOP is not supported");
            return MP4_STATUS_PARSE_ERROR;
        }
    }
    if (VOL->shape != MP4_SHAPE_TYPE_RECTANGULAR) {
        if (!(VOL->sprite_enable == MP4_SPRITE_STATIC && VOP->coding_type == MP4_VOP_TYPE_I)) {
            VOP->vop_width = mp4_GetBits(pInfo, 13);
            if (VOP->vop_width == 0) {
                mp4_Error("Error: wrong value of vop_width");
                return MP4_STATUS_PARSE_ERROR;
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOP->vop_height = mp4_GetBits(pInfo, 13);
            if (VOP->vop_height == 0) {
                mp4_Error("Error: wrong value of vop_height");
                return MP4_STATUS_PARSE_ERROR;
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOP->vop_horizontal_mc_spatial_ref = mp4_GetBits(pInfo, 13);
            VOP->vop_horizontal_mc_spatial_ref <<= (32 - 13);
            VOP->vop_horizontal_mc_spatial_ref >>= (32 - 13);
            if (VOP->vop_horizontal_mc_spatial_ref & 1) {
                mp4_Error("Error: vop_horizontal_mc_spatial_ref must be divisible by 2");
                return MP4_STATUS_PARSE_ERROR;
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
            VOP->vop_vertical_mc_spatial_ref = mp4_GetBits(pInfo, 13);
            VOP->vop_vertical_mc_spatial_ref <<= (32 - 13);
            VOP->vop_vertical_mc_spatial_ref >>= (32 - 13);
            if (VOP->vop_vertical_mc_spatial_ref & 1) {
                mp4_Error("Error: vop_vertical_mc_spatial_ref must be divisible by 2");
                return MP4_STATUS_PARSE_ERROR;
            }
            if (!mp4_GetMarkerBit(pInfo)) return MP4_STATUS_PARSE_ERROR;
        }
//f        if ((VOL->shape != MP4_SHAPE_TYPE_BINARYONLY) && VOL->scalability && enhancement_type)
//f            background_composition = mp4_GetBit(pInfo);
        VOP->change_conv_ratio_disable = mp4_GetBit(pInfo);
        VOP->vop_constant_alpha = mp4_GetBit(pInfo);
        if (VOP->vop_constant_alpha)
            VOP->vop_constant_alpha_value = mp4_GetBits9(pInfo, 8);
        else
            VOP->vop_constant_alpha_value = 255;
    }
    if (VOL->shape != MP4_SHAPE_TYPE_BINARYONLY) {
        if (!VOL->complexity_estimation_disable) {
            if (VOL->ComplexityEstimation.estimation_method == 0) {
                if (VOP->coding_type == MP4_VOP_TYPE_I) {
                    if (VOL->ComplexityEstimation.opaque) VOL->ComplexityEstimation.dcecs_opaque =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.transparent) VOL->ComplexityEstimation.dcecs_transparent =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.intra_cae) VOL->ComplexityEstimation.dcecs_intra_cae =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.inter_cae) VOL->ComplexityEstimation.dcecs_inter_cae =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.no_update) VOL->ComplexityEstimation.dcecs_no_update =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.upsampling) VOL->ComplexityEstimation.dcecs_upsampling =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.intra_blocks) VOL->ComplexityEstimation.dcecs_intra_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.not_coded_blocks) VOL->ComplexityEstimation.dcecs_not_coded_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_coefs) VOL->ComplexityEstimation.dcecs_dct_coefs =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_lines) VOL->ComplexityEstimation.dcecs_dct_lines =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_symbols) VOL->ComplexityEstimation.dcecs_vlc_symbols =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_bits) VOL->ComplexityEstimation.dcecs_vlc_bits =  mp4_GetBits9(pInfo, 4);
                    if (VOL->ComplexityEstimation.sadct) VOL->ComplexityEstimation.dcecs_sadct =  mp4_GetBits9(pInfo, 8);
                }
                if (VOP->coding_type == MP4_VOP_TYPE_P) {
                    if (VOL->ComplexityEstimation.opaque) VOL->ComplexityEstimation.dcecs_opaque =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.transparent) VOL->ComplexityEstimation.dcecs_transparent =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.intra_cae) VOL->ComplexityEstimation.dcecs_intra_cae =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.inter_cae) VOL->ComplexityEstimation.dcecs_inter_cae =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.no_update) VOL->ComplexityEstimation.dcecs_no_update =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.upsampling) VOL->ComplexityEstimation.dcecs_upsampling =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.intra_blocks) VOL->ComplexityEstimation.dcecs_intra_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.not_coded_blocks) VOL->ComplexityEstimation.dcecs_not_coded_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_coefs) VOL->ComplexityEstimation.dcecs_dct_coefs =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_lines) VOL->ComplexityEstimation.dcecs_dct_lines =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_symbols) VOL->ComplexityEstimation.dcecs_vlc_symbols =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_bits) VOL->ComplexityEstimation.dcecs_vlc_bits =  mp4_GetBits9(pInfo, 4);
                    if (VOL->ComplexityEstimation.inter_blocks) VOL->ComplexityEstimation.dcecs_inter_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.inter4v_blocks) VOL->ComplexityEstimation.dcecs_inter4v_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.apm) VOL->ComplexityEstimation.dcecs_apm =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.npm) VOL->ComplexityEstimation.dcecs_npm =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.forw_back_mc_q) VOL->ComplexityEstimation.dcecs_forw_back_mc_q =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.halfpel2) VOL->ComplexityEstimation.dcecs_halfpel2 =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.halfpel4) VOL->ComplexityEstimation.dcecs_halfpel4 =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.sadct) VOL->ComplexityEstimation.dcecs_sadct =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.quarterpel) VOL->ComplexityEstimation.dcecs_quarterpel =  mp4_GetBits9(pInfo, 8);
                }
                if (VOP->coding_type == MP4_VOP_TYPE_B) {
                    if (VOL->ComplexityEstimation.opaque) VOL->ComplexityEstimation.dcecs_opaque =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.transparent) VOL->ComplexityEstimation.dcecs_transparent =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.intra_cae) VOL->ComplexityEstimation.dcecs_intra_cae =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.inter_cae) VOL->ComplexityEstimation.dcecs_inter_cae =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.no_update) VOL->ComplexityEstimation.dcecs_no_update =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.upsampling) VOL->ComplexityEstimation.dcecs_upsampling =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.intra_blocks) VOL->ComplexityEstimation.dcecs_intra_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.not_coded_blocks) VOL->ComplexityEstimation.dcecs_not_coded_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_coefs) VOL->ComplexityEstimation.dcecs_dct_coefs =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_lines) VOL->ComplexityEstimation.dcecs_dct_lines =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_symbols) VOL->ComplexityEstimation.dcecs_vlc_symbols =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_bits) VOL->ComplexityEstimation.dcecs_vlc_bits =  mp4_GetBits9(pInfo, 4);
                    if (VOL->ComplexityEstimation.inter_blocks) VOL->ComplexityEstimation.dcecs_inter_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.inter4v_blocks) VOL->ComplexityEstimation.dcecs_inter4v_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.apm) VOL->ComplexityEstimation.dcecs_apm =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.npm) VOL->ComplexityEstimation.dcecs_npm =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.forw_back_mc_q) VOL->ComplexityEstimation.dcecs_forw_back_mc_q =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.halfpel2) VOL->ComplexityEstimation.dcecs_halfpel2 =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.halfpel4) VOL->ComplexityEstimation.dcecs_halfpel4 =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.interpolate_mc_q) VOL->ComplexityEstimation.dcecs_interpolate_mc_q =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.sadct) VOL->ComplexityEstimation.dcecs_sadct =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.quarterpel) VOL->ComplexityEstimation.dcecs_quarterpel =  mp4_GetBits9(pInfo, 8);
                }
                if (VOP->coding_type == MP4_VOP_TYPE_S && VOL->sprite_enable == MP4_SPRITE_STATIC) {
                    if (VOL->ComplexityEstimation.intra_blocks) VOL->ComplexityEstimation.dcecs_intra_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.not_coded_blocks) VOL->ComplexityEstimation.dcecs_not_coded_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_coefs) VOL->ComplexityEstimation.dcecs_dct_coefs =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.dct_lines) VOL->ComplexityEstimation.dcecs_dct_lines =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_symbols) VOL->ComplexityEstimation.dcecs_vlc_symbols =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.vlc_bits) VOL->ComplexityEstimation.dcecs_vlc_bits =  mp4_GetBits9(pInfo, 4);
                    if (VOL->ComplexityEstimation.inter_blocks) VOL->ComplexityEstimation.dcecs_inter_blocks =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.inter4v_blocks) VOL->ComplexityEstimation.dcecs_inter4v_blocks =  mp4_GetBits(pInfo, 8);
                    if (VOL->ComplexityEstimation.apm) VOL->ComplexityEstimation.dcecs_apm =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.npm) VOL->ComplexityEstimation.dcecs_npm =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.forw_back_mc_q) VOL->ComplexityEstimation.dcecs_forw_back_mc_q =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.halfpel2) VOL->ComplexityEstimation.dcecs_halfpel2 =  mp4_GetBits9(pInfo, 8);
                    if (VOL->ComplexityEstimation.halfpel4) VOL->ComplexityEstimation.dcecs_halfpel4 =  mp4_GetBits(pInfo, 8);
                    if (VOL->ComplexityEstimation.interpolate_mc_q) VOL->ComplexityEstimation.dcecs_interpolate_mc_q =  mp4_GetBits9(pInfo, 8);
                }
            }
        }
        VOP->intra_dc_vlc_thr = mp4_GetBits9(pInfo, 3);
        if (VOL->interlaced) {
            VOP->top_field_first = mp4_GetBit(pInfo);
            VOP->alternate_vertical_scan_flag = mp4_GetBit(pInfo);
        }
    }
    if ((VOL->sprite_enable == MP4_SPRITE_STATIC || VOL->sprite_enable == MP4_SPRITE_GMC) && VOP->coding_type == MP4_VOP_TYPE_S) {
        if (VOL->sprite_warping_points > 0)
            if (mp4_Sprite_Trajectory(pInfo) != MP4_STATUS_OK)
                return MP4_STATUS_PARSE_ERROR;
        if (VOL->sprite_brightness_change) {
            code = mp4_ShowBits9(pInfo, 4);
            if (code == 15) {
                mp4_FlushBits(pInfo, 4);
                VOP->brightness_change_factor = 625 + mp4_GetBits(pInfo, 10);
            } else if (code == 14) {
                mp4_FlushBits(pInfo, 4);
                VOP->brightness_change_factor = 113 + mp4_GetBits9(pInfo, 9);
            } else if (code >= 12) {
                mp4_FlushBits(pInfo, 3);
                code = mp4_GetBits9(pInfo, 7);
                VOP->brightness_change_factor = (code < 64) ? code - 112 : code - 15;
            } else if (code >= 8) {
                mp4_FlushBits(pInfo, 2);
                code = mp4_GetBits9(pInfo, 6);
                VOP->brightness_change_factor = (code < 32) ? code - 48 : code - 15;
            } else {
                mp4_FlushBits(pInfo, 1);
                code = mp4_GetBits9(pInfo, 5);
                VOP->brightness_change_factor = (code < 16) ? code - 16 : code - 15;
            }
        } else
            VOP->brightness_change_factor = 0;
        if (VOL->sprite_enable == MP4_SPRITE_STATIC)
            return MP4_STATUS_OK;
    }
    if (VOL->shape != MP4_SHAPE_TYPE_BINARYONLY) {
        VOP->quant = mp4_GetBits9(pInfo, VOL->quant_precision);
        if (VOL->shape == MP4_SHAPE_TYPE_GRAYSCALE) {
            int32_t   ac, i;

            ac = mp4_aux_comp_count[VOL->shape_extension];
            for (i = 0; i < ac; i ++)
                VOP->alpha_quant[i] = mp4_GetBits9(pInfo, 6);
        }
        if (VOP->coding_type != MP4_VOP_TYPE_I) {
            VOP->fcode_forward = mp4_GetBits9(pInfo, 3);
            if (VOP->fcode_forward == 0) {
                mp4_Error("Error: vop_fcode_forward == 0");
                return MP4_STATUS_PARSE_ERROR;
            }
        }
        if (VOP->coding_type == MP4_VOP_TYPE_B) {
            VOP->fcode_backward = mp4_GetBits9(pInfo, 3);
            if (VOP->fcode_backward == 0) {
                mp4_Error("Error: vop_fcode_backward == 0");
                return MP4_STATUS_PARSE_ERROR;
            }
        }
        if (!VOL->scalability) {
            if (VOL->shape != MP4_SHAPE_TYPE_RECTANGULAR && VOP->coding_type != MP4_VOP_TYPE_I)
                VOP->shape_coding_type = mp4_GetBit(pInfo);
        } else {
            //f if (VOL->enhancement_type) {
                //f VOP->load_backward_shape = mp4_GetBit(pInfo);
                //f if (VOP->load_backward_shape) {
                    //f shape
                //f }
            //f }
            VOP->ref_select_code = mp4_GetBits9(pInfo, 2);
        }
    }
    return MP4_STATUS_OK;
}

/*
// decode VideoPacket
*/
mp4_Status mp4_CheckDecodeVideoPacket(mp4_Info* pInfo, int32_t *found)
{
    uint32_t      code;
    int32_t      header_extension_code, rml;
    mp4_VideoObject       *VOL = &pInfo->VisualObject.VideoObject;
    mp4_VideoObjectPlane  *VOP = &pInfo->VisualObject.VideoObject.VideoObjectPlane;

    *found = 0;
    // check stuffing bits
    if (mp4_ShowBit(pInfo) != 0)
        return MP4_STATUS_OK;
    if (mp4_ShowBits9(pInfo, 8 - pInfo->bitoff) != (uint32_t)((1 << (7 - pInfo->bitoff)) - 1))
        return MP4_STATUS_OK;
    if (VOP->coding_type == MP4_VOP_TYPE_I)
        rml = 17;
    else if (VOP->coding_type == MP4_VOP_TYPE_B)
        rml = 16 + IPP_MAX(VOP->fcode_forward, VOP->fcode_backward);
    else
        rml = 16 + VOP->fcode_forward;
    code = (pInfo->bufptr[1] << 16) | (pInfo->bufptr[2] << 8) | pInfo->bufptr[3];
    code >>= 24 - rml;
    if (code == 1) { // is resync_marker
        mp4_FlushBits(pInfo, rml + 8 - pInfo->bitoff);
        header_extension_code = 0;
        if (VOL->shape != MP4_SHAPE_TYPE_RECTANGULAR) {
            header_extension_code = mp4_GetBit(pInfo);
            if (header_extension_code && !(VOL->sprite_enable == MP4_SPRITE_STATIC && VOP->coding_type == MP4_VOP_TYPE_I)) {
                VOP->vop_width = mp4_GetBits(pInfo, 13);
                if (!mp4_GetBit(pInfo))
                    if (pInfo->stopOnErr)
                        goto Err;
                VOP->vop_height = mp4_GetBits(pInfo, 13);
                if (!mp4_GetBit(pInfo))
                    if (pInfo->stopOnErr)
                        goto Err;
                VOP->vop_horizontal_mc_spatial_ref = mp4_GetBits(pInfo, 13);
                if (!mp4_GetBit(pInfo))
                    if (pInfo->stopOnErr)
                        goto Err;
                VOP->vop_vertical_mc_spatial_ref = mp4_GetBits(pInfo, 13);
                if (!mp4_GetBit(pInfo))
                    if (pInfo->stopOnErr)
                        goto Err;
            }
        }
        pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num = mp4_GetBits(pInfo, pInfo->VisualObject.VideoObject.mbns);
        if (pInfo->VisualObject.VideoObject.VideoObjectPlane.macroblock_num >= pInfo->VisualObject.VideoObject.MacroBlockPerVOP)
            goto Err;
        if (VOL->shape != MP4_SHAPE_TYPE_BINARYONLY) {
            pInfo->VisualObject.VideoObject.VideoObjectPlane.quant_scale = mp4_GetBits9(pInfo, VOL->quant_precision); // quant_scale
        }
        if (VOL->shape == MP4_SHAPE_TYPE_RECTANGULAR) {
            header_extension_code = mp4_GetBit(pInfo);
        }
        if (header_extension_code) {
            //f ignore modulo_time_base
            do {
                code = mp4_GetBit(pInfo);
            } while (code);
            if (!mp4_GetBit(pInfo))
                if (pInfo->stopOnErr)
                    goto Err;
            //f ignore vop_time_increment
            if (VOL->vop_time_increment_resolution_bits != 0 ) {
                code = mp4_GetBits(pInfo, VOL->vop_time_increment_resolution_bits);
            }
            if (!mp4_GetBit(pInfo))
                if (pInfo->stopOnErr)
                    goto Err;
            //f ignore vop_coding_type
            code = mp4_GetBits9(pInfo, 2);
            if (VOL->shape != MP4_SHAPE_TYPE_RECTANGULAR) {
                VOP->change_conv_ratio_disable = mp4_GetBit(pInfo);
                if (VOP->coding_type != MP4_VOP_TYPE_I)
                    VOP->shape_coding_type = mp4_GetBit(pInfo);
            }
            if (VOL->shape != MP4_SHAPE_TYPE_BINARYONLY) {
                //f ignore intra_dc_vlc_thr
                code = mp4_GetBits9(pInfo, 3);
                if (VOL->sprite_enable == MP4_SPRITE_GMC && VOP->coding_type == MP4_VOP_TYPE_S && VOL->sprite_warping_points > 0)
                    if (mp4_Sprite_Trajectory(pInfo) != MP4_STATUS_OK)
                        if (pInfo->stopOnErr)
                            goto Err;
                //f ignore vop_reduced_resolution
                if (VOL->reduced_resolution_vop_enable && VOL->shape == MP4_SHAPE_TYPE_RECTANGULAR &&
                    (VOP->coding_type == MP4_VOP_TYPE_I || VOP->coding_type == MP4_VOP_TYPE_P))
                    code = mp4_GetBit(pInfo);
                if (VOP->coding_type != MP4_VOP_TYPE_I)
                    VOP->fcode_forward = mp4_GetBits9(pInfo, 3);
                if (VOP->coding_type == MP4_VOP_TYPE_B)
                    VOP->fcode_backward = mp4_GetBits9(pInfo, 3);
            }
        }
        if (VOL->newpred_enable) {
            int vop_id_length = VOL->vop_time_increment_resolution_bits + 3;
            if (vop_id_length > 15)
                vop_id_length = 15;
            VOP->vop_id = mp4_GetBits(pInfo, vop_id_length);
            VOP->vop_id_for_prediction_indication = mp4_GetBit(pInfo);
            if (VOP->vop_id_for_prediction_indication)
                VOP->vop_id_for_prediction = mp4_GetBits(pInfo, vop_id_length);
            if (!mp4_GetMarkerBit(pInfo))
                if (pInfo->stopOnErr)
                    goto Err;
        }
        *found = 1;
        return MP4_STATUS_OK;
    }
    return MP4_STATUS_OK;
Err:
    mp4_Error("Error: decoding Video Packet Header");
    return MP4_STATUS_PARSE_ERROR;
}

int32_t mp4_CheckDecodeGOB_SVH(mp4_Info* pInfo)
{
    int32_t  code;

    pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number ++;
    pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_header_empty = 1;
    code = mp4_ShowBits(pInfo, 17); /* check gob_resync_marker */
    if (code != 1) {
        code = mp4_ShowBitsAlign(pInfo, 17); /* check next aligned bits are gob_resync_marker */
        if (code == 1)
            mp4_AlignBits(pInfo);
    }
    if (code == 1) {
        mp4_FlushBits(pInfo, 17);
        pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_header_empty = 0;
        code = mp4_GetBits9(pInfo, 5); /* gob_number */
        pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_frame_id = mp4_GetBits9(pInfo, 2);
        pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.quant_scale = mp4_GetBits9(pInfo, 5);
        // check gob_number is valid
        //if (code > pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number && code <= pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_gobs_in_vop)
        if (code <= pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.num_gobs_in_vop)
            pInfo->VisualObject.VideoObject.VideoObjectPlaneH263.gob_number = code;
        else {
            mp4_Error("Error: bad gob_number in GOB header");
            return MP4_STATUS_ERROR;
        }
    }
    return MP4_STATUS_OK;
}

