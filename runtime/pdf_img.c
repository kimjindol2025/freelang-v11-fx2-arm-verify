// pdf_img.c — FreeLang fx PDF JPEG 이미지 임베딩
// FL API:
//   pdf-img-load path → {"w":w "h":h "data":hex "size":n} 또는 nil
//   pdf-img-obj  img-map obj-id → PDF XObject 문자열

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// JPEG SOF 마커에서 w/h 추출
static int jpeg_dimensions(const uint8_t *data, size_t size, int *w, int *h) {
    for (size_t i = 0; i + 4 < size; i++) {
        if (data[i] != 0xFF) continue;
        uint8_t m = data[i+1];
        if (m == 0xC0 || m == 0xC1 || m == 0xC2) {
            if (i + 9 >= size) return 0;
            *h = (data[i+5]<<8)|data[i+6];
            *w = (data[i+7]<<8)|data[i+8];
            return 1;
        }
        if (m == 0xD8 || m == 0xD9 || m == 0x00) { i++; continue; }
        if (i + 3 < size) {
            uint16_t len = (data[i+2]<<8)|data[i+3];
            i += 1 + len;
        }
    }
    return 0;
}

// JPEG 파일 로드 → FL map {"w" "h" "hex" "size"}
FLValue pdf_img_load(FLValue path_v) {
    if (path_v.tag != FL_STRING || !path_v.obj) return fl_nil();
    const char *path = ((FLString*)path_v.obj)->data;

    FILE *fp = fopen(path, "rb");
    if (!fp) { fprintf(stderr, "[pdf_img] 파일 열기 실패: %s\n", path); return fl_nil(); }
    fseek(fp, 0, SEEK_END); size_t sz = ftell(fp); rewind(fp);
    uint8_t *data = malloc(sz);
    if (!data) { fclose(fp); return fl_nil(); }
    fread(data, 1, sz, fp); fclose(fp);

    int w = 0, h = 0;
    if (!jpeg_dimensions(data, sz, &w, &h)) {
        fprintf(stderr, "[pdf_img] JPEG 파싱 실패: %s\n", path);
        free(data); return fl_nil();
    }

    // hex 인코딩
    char *hex = malloc(sz * 2 + 2);
    if (!hex) { free(data); return fl_nil(); }
    for (size_t i = 0; i < sz; i++)
        snprintf(hex + i*2, 3, "%02X", data[i]);
    hex[sz*2] = '>'; hex[sz*2+1] = 0;
    free(data);

    FLValue result = fl_map_new();
    result = fl_map_set(result, fl_str_val("w"),    fl_int(w));
    result = fl_map_set(result, fl_str_val("h"),    fl_int(h));
    result = fl_map_set(result, fl_str_val("hex"),  fl_str_val(hex));
    result = fl_map_set(result, fl_str_val("size"), fl_int((long long)sz));
    free(hex);
    return result;
}

// PDF XObject 생성
// obj_id: 이 이미지 오브젝트의 ID
// 반환: PDF 오브젝트 문자열
FLValue pdf_img_obj(FLValue img_v, FLValue obj_id_v) {
    if (img_v.tag != FL_MAP || !img_v.obj) return fl_str_val("");
    int oid = (obj_id_v.tag == FL_INT) ? (int)obj_id_v.i : 100;

    FLValue w_v    = fl_map_get(img_v, fl_str_val("w"));
    FLValue h_v    = fl_map_get(img_v, fl_str_val("h"));
    FLValue hex_v  = fl_map_get(img_v, fl_str_val("hex"));
    FLValue size_v = fl_map_get(img_v, fl_str_val("size"));

    if (w_v.tag != FL_INT || h_v.tag != FL_INT) return fl_str_val("");
    int w = (int)w_v.i, h = (int)h_v.i;
    long long sz = (size_v.tag == FL_INT) ? size_v.i : 0;
    const char *hex = (hex_v.tag == FL_STRING && hex_v.obj) ? ((FLString*)hex_v.obj)->data : "";
    size_t hexlen = strlen(hex);

    // hex 길이 = sz*2 + 1 (마지막 '>')
    char hdr[400];
    snprintf(hdr, sizeof(hdr),
        "%d 0 obj\n"
        "<< /Type /XObject /Subtype /Image\n"
        "   /Width %d /Height %d\n"
        "   /ColorSpace /DeviceRGB /BitsPerComponent 8\n"
        "   /Filter [/ASCIIHexDecode /DCTDecode]\n"
        "   /Length %lld >>\nstream\n",
        oid, w, h, sz*2+1);

    const char *ftr = "\nendstream\nendobj\n";
    size_t total = strlen(hdr) + hexlen + strlen(ftr);
    char *result = malloc(total + 4);
    if (!result) return fl_str_val("");
    size_t ri = 0;
    memcpy(result+ri, hdr, strlen(hdr)); ri += strlen(hdr);
    memcpy(result+ri, hex, hexlen);      ri += hexlen;
    memcpy(result+ri, ftr, strlen(ftr)); ri += strlen(ftr);
    result[ri] = 0;

    FLValue r = fl_str_val(result);
    free(result);
    return r;
}
