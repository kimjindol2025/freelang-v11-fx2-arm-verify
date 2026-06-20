// pdf_ttf.c — FreeLang fx PDF 한글 폰트 임베딩 (bigendian 수정)
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ── 빅엔디안 읽기 (항상 r16/r32 사용 — 직접 캐스팅 금지) ──────────────

static uint16_t r16(const uint8_t *p) { return ((uint16_t)p[0]<<8)|p[1]; }
static uint32_t r32(const uint8_t *p) { return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }

// ── TTF 구조 ─────────────────────────────────────────────────────────────

typedef struct {
    uint8_t  *data;
    size_t    size;
    // cmap format4 세그먼트 (네이티브 엔디안으로 변환 저장)
    uint16_t *f4_start;
    uint16_t *f4_end;
    int16_t  *f4_delta;
    uint16_t *f4_range_off; // range_off 원본값 (바이트 오프셋 계산용)
    uint32_t  f4_range_arr_offset; // glyph_ids 배열의 TTF 내 오프셋
    int       f4_seg;
    // cmap format12
    uint32_t *f12_start;
    uint32_t *f12_end;
    uint32_t *f12_glyph;
    int       f12_groups;
    // hmtx
    uint16_t *hmtx_aw;
    int       num_hmtx;
    uint16_t  num_glyphs;
} TTFFont;

static uint32_t find_table(const uint8_t *ttf, size_t size, const char tag[4]) {
    if (size < 12) return 0;
    uint16_t n = r16(ttf + 4);
    for (int i = 0; i < n; i++) {
        const uint8_t *rec = ttf + 12 + i * 16;
        if (rec + 16 > ttf + size) break;
        if (memcmp(rec, tag, 4) == 0) return r32(rec + 8);
    }
    return 0;
}

static void parse_cmap(TTFFont *f) {
    uint32_t cmap_off = find_table(f->data, f->size, "cmap");
    if (!cmap_off || cmap_off + 4 > f->size) return;
    const uint8_t *cmap = f->data + cmap_off;
    uint16_t nsub = r16(cmap + 2);
    uint32_t f4_off = 0, f12_off = 0;
    for (int i = 0; i < nsub; i++) {
        const uint8_t *enc = cmap + 4 + i * 8;
        if (enc + 8 > f->data + f->size) break;
        uint16_t platform = r16(enc);
        uint32_t off = r32(enc + 4);
        const uint8_t *sub = cmap + off;
        if (sub + 2 > f->data + f->size) continue;
        uint16_t fmt = r16(sub);
        if (fmt == 4 && !f4_off && (platform == 0 || platform == 3)) f4_off = off;
        if (fmt == 12 && !f12_off && (platform == 0 || platform == 3)) f12_off = off;
    }

    // format4 — 각 값을 r16으로 읽어 네이티브 배열에 저장
    if (f4_off) {
        const uint8_t *s = cmap + f4_off;
        if (s + 14 > f->data + f->size) goto try_f12;
        int seg = r16(s + 6) / 2;
        f->f4_seg     = seg;
        f->f4_end     = malloc(seg * sizeof(uint16_t));
        f->f4_start   = malloc(seg * sizeof(uint16_t));
        f->f4_delta   = malloc(seg * sizeof(int16_t));
        f->f4_range_off = malloc(seg * sizeof(uint16_t));
        const uint8_t *end_arr     = s + 14;
        const uint8_t *start_arr   = s + 14 + seg * 2 + 2; // +2 for reservedPad
        const uint8_t *delta_arr   = start_arr + seg * 2;
        const uint8_t *range_arr   = delta_arr + seg * 2;
        f->f4_range_arr_offset = (uint32_t)(range_arr - f->data);
        for (int i = 0; i < seg; i++) {
            f->f4_end[i]       = r16(end_arr   + i * 2);
            f->f4_start[i]     = r16(start_arr + i * 2);
            f->f4_delta[i]     = (int16_t)r16(delta_arr + i * 2);
            f->f4_range_off[i] = r16(range_arr + i * 2);
        }
    }

    try_f12:
    // format12 — 완전 유니코드 커버
    if (f12_off) {
        const uint8_t *s = cmap + f12_off;
        if (s + 16 > f->data + f->size) return;
        uint32_t groups = r32(s + 12);
        if (groups > 65535) return;
        f->f12_start  = malloc(groups * sizeof(uint32_t));
        f->f12_end    = malloc(groups * sizeof(uint32_t));
        f->f12_glyph  = malloc(groups * sizeof(uint32_t));
        f->f12_groups = (int)groups;
        for (uint32_t i = 0; i < groups; i++) {
            const uint8_t *g = s + 16 + i * 12;
            if (g + 12 > f->data + f->size) { f->f12_groups = i; break; }
            f->f12_start[i] = r32(g);
            f->f12_end[i]   = r32(g + 4);
            f->f12_glyph[i] = r32(g + 8);
        }
    }
}

static void parse_hmtx(TTFFont *f) {
    uint32_t hhea_off = find_table(f->data, f->size, "hhea");
    uint32_t hmtx_off = find_table(f->data, f->size, "hmtx");
    if (!hhea_off || !hmtx_off) return;
    int nhm = r16(f->data + hhea_off + 34);
    f->hmtx_aw  = malloc(f->num_glyphs * sizeof(uint16_t));
    f->num_hmtx = f->num_glyphs;
    uint16_t last_aw = 1000;
    for (int i = 0; i < f->num_glyphs; i++) {
        if (i < nhm) {
            uint32_t off = hmtx_off + i * 4;
            if (off + 2 <= f->size) last_aw = r16(f->data + off);
        }
        f->hmtx_aw[i] = last_aw;
    }
}

static uint32_t unicode_to_glyph(const TTFFont *f, uint32_t cp) {
    // format12 우선
    for (int i = 0; i < f->f12_groups; i++) {
        if (cp >= f->f12_start[i] && cp <= f->f12_end[i])
            return f->f12_glyph[i] + (cp - f->f12_start[i]);
    }
    // format4 (BMP)
    if (cp <= 0xFFFF && f->f4_seg > 0) {
        for (int i = 0; i < f->f4_seg; i++) {
            if (cp > f->f4_end[i]) continue;
            if (cp < f->f4_start[i]) return 0;
            uint16_t ro = f->f4_range_off[i];
            if (ro == 0) {
                return (uint16_t)((int32_t)cp + f->f4_delta[i]);
            } else {
                // glyph_ids 배열 인덱스 계산
                // range_off[i]가 가리키는 위치: &range_off[i] + ro
                uint32_t glyph_arr_pos = f->f4_range_arr_offset + i * 2 + ro + (cp - f->f4_start[i]) * 2;
                if (glyph_arr_pos + 2 > f->size) return 0;
                uint16_t gid = r16(f->data + glyph_arr_pos);
                if (gid == 0) return 0;
                return (uint16_t)((int32_t)gid + f->f4_delta[i]);
            }
        }
    }
    return 0;
}

// ── UTF-8 디코딩 ─────────────────────────────────────────────────────────

static int utf8_next(const uint8_t **p, const uint8_t *end, uint32_t *cp) {
    const uint8_t *s = *p;
    if (s >= end) return 0;
    uint8_t c = *s++;
    if      (c < 0x80) { *cp = c; }
    else if (c < 0xE0) { if (s >= end) return 0; *cp = ((c&0x1F)<<6)|(*s++&0x3F); }
    else if (c < 0xF0) { if (s+1 >= end) return 0; *cp = ((c&0x0F)<<12)|((*s&0x3F)<<6)|(*(s+1)&0x3F); s+=2; }
    else               { if (s+2 >= end) return 0; *cp = ((c&0x07)<<18)|((*s&0x3F)<<12)|((*(s+1)&0x3F)<<6)|(*(s+2)&0x3F); s+=3; }
    *p = s;
    return 1;
}

// ── 전역 폰트 캐시 ───────────────────────────────────────────────────────

static TTFFont *g_font = NULL;
static char     g_font_path[1024] = "";

static void free_font(TTFFont *f) {
    if (!f) return;
    free(f->data);
    free(f->f4_start); free(f->f4_end); free(f->f4_delta); free(f->f4_range_off);
    free(f->f12_start); free(f->f12_end); free(f->f12_glyph);
    free(f->hmtx_aw);
    free(f);
}

// ── FL API ───────────────────────────────────────────────────────────────

FLValue pdf_ttf_load(FLValue path_v) {
    if (path_v.tag != FL_STRING || !path_v.obj) return fl_nil();
    const char *path = ((FLString*)path_v.obj)->data;
    if (strcmp(path, g_font_path) == 0 && g_font) return fl_bool(1);

    FILE *fp = fopen(path, "rb");
    if (!fp) { fprintf(stderr, "[pdf_ttf] 폰트 열기 실패: %s\n", path); return fl_nil(); }
    fseek(fp, 0, SEEK_END); size_t sz = ftell(fp); rewind(fp);
    uint8_t *data = malloc(sz);
    if (!data) { fclose(fp); return fl_nil(); }
    fread(data, 1, sz, fp); fclose(fp);

    free_font(g_font);
    g_font = calloc(1, sizeof(TTFFont));
    g_font->data = data; g_font->size = sz;

    uint32_t maxp_off = find_table(data, sz, "maxp");
    g_font->num_glyphs = maxp_off ? r16(data + maxp_off + 4) : 65535;

    parse_cmap(g_font);
    parse_hmtx(g_font);
    strncpy(g_font_path, path, sizeof(g_font_path)-1);

    fprintf(stderr, "[pdf_ttf] 로드: %s (%zuKB, glyphs=%d, f4seg=%d, f12grp=%d)\n",
            path, sz/1024, g_font->num_glyphs, g_font->f4_seg, g_font->f12_groups);
    return fl_bool(1);
}

// UTF-8 → PDF Identity-H hex string "<GGGG...>"
FLValue pdf_ttf_glyphs(FLValue text_v) {
    if (!g_font || text_v.tag != FL_STRING || !text_v.obj) return fl_str_val("<>");
    const char *txt = ((FLString*)text_v.obj)->data;
    size_t tlen = ((FLString*)text_v.obj)->len;
    const uint8_t *p = (const uint8_t*)txt;
    const uint8_t *end = p + tlen;

    char *out = malloc(tlen * 5 + 4);
    if (!out) return fl_str_val("<>");
    out[0] = '<'; int oi = 1;
    uint32_t cp;
    while (utf8_next(&p, end, &cp)) {
        uint32_t gid = unicode_to_glyph(g_font, cp);
        oi += snprintf(out + oi, 6, "%04X", (unsigned)(gid & 0xFFFF));
    }
    out[oi++] = '>'; out[oi] = 0;
    FLValue r = fl_str_val(out);
    free(out);
    return r;
}

// PDF 폰트 오브젝트 4개 생성 (ASCIIHex 인코딩)
FLValue pdf_ttf_embed_objs(FLValue base_id_v) {
    if (!g_font) return fl_str_val("");
    int base = (base_id_v.tag == FL_INT) ? (int)base_id_v.i : 10;
    int stream_id  = base;
    int fdesc_id   = base + 1;
    int cidfont_id = base + 2;
    int type0_id   = base + 3;

    size_t fsz = g_font->size;

    // hex 인코딩
    char *hex = malloc(fsz * 2 + 2);
    if (!hex) return fl_nil();
    for (size_t i = 0; i < fsz; i++)
        snprintf(hex + i*2, 3, "%02X", g_font->data[i]);
    hex[fsz*2] = '>'; hex[fsz*2+1] = 0;

    // 헤더 (길이 = hex chars + '>')
    char hdr1[300];
    snprintf(hdr1, sizeof(hdr1),
        "%d 0 obj\n<< /Length %zu /Length1 %zu /Filter /ASCIIHexDecode >>\nstream\n",
        stream_id, fsz*2+1, fsz);

    char obj2[512];
    snprintf(obj2, sizeof(obj2),
        "%d 0 obj\n"
        "<< /Type /FontDescriptor /FontName /NanumGothic\n"
        "   /Flags 32 /FontBBox [-1000 -1000 2000 2000]\n"
        "   /ItalicAngle 0 /Ascent 800 /Descent -200 /StemV 80\n"
        "   /FontFile2 %d 0 R >>\nendobj\n",
        fdesc_id, stream_id);

    char obj3[512];
    snprintf(obj3, sizeof(obj3),
        "%d 0 obj\n"
        "<< /Type /Font /Subtype /CIDFontType2 /BaseFont /NanumGothic\n"
        "   /CIDSystemInfo << /Registry (Adobe) /Ordering (Identity) /Supplement 0 >>\n"
        "   /FontDescriptor %d 0 R /DW 1000 >>\nendobj\n",
        cidfont_id, fdesc_id);

    char obj4[512];
    snprintf(obj4, sizeof(obj4),
        "%d 0 obj\n"
        "<< /Type /Font /Subtype /Type0 /BaseFont /NanumGothic\n"
        "   /Encoding /Identity-H /DescendantFonts [%d 0 R] >>\nendobj\n",
        type0_id, cidfont_id);

    size_t total = strlen(hdr1) + fsz*2+1 + strlen("\nendstream\nendobj\n") + strlen(obj2) + strlen(obj3) + strlen(obj4);
    char *result = malloc(total + 8);
    if (!result) { free(hex); return fl_nil(); }
    size_t ri = 0;
    memcpy(result+ri, hdr1, strlen(hdr1)); ri += strlen(hdr1);
    memcpy(result+ri, hex, fsz*2+1);       ri += fsz*2+1;
    free(hex);
    const char *ftr = "\nendstream\nendobj\n";
    memcpy(result+ri, ftr, strlen(ftr)); ri += strlen(ftr);
    memcpy(result+ri, obj2, strlen(obj2)); ri += strlen(obj2);
    memcpy(result+ri, obj3, strlen(obj3)); ri += strlen(obj3);
    memcpy(result+ri, obj4, strlen(obj4)); ri += strlen(obj4);
    result[ri] = 0;

    FLValue r = fl_str_val(result);
    free(result);
    return r;
}

FLValue pdf_ttf_size(void) {
    return fl_int(g_font ? (long long)g_font->size : 0LL);
}
