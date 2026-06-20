/*
 * smtp.c — 메일 발송 (sendmail popen)
 *
 * smtp_send(from, to, subject, body)      → true/false
 * smtp_send_html(from, to, subject, html) → true/false
 * smtp_test(to)                           → true/false (테스트 메일)
 */
#include "runtime.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static inline const char* smtp_cstr(FLValue v) {
    if (v.tag != FL_STRING || !v.obj) return "";
    return ((FLString*)v.obj)->data;
}

static void make_date_header(char* buf, size_t sz) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    strftime(buf, sz, "%a, %d %b %Y %H:%M:%S %z", tm);
}

static void make_msgid(char* buf, size_t sz, const char* from) {
    time_t t = time(NULL);
    const char* at = strchr(from, '@');
    const char* domain = at ? at + 1 : "localhost";
    snprintf(buf, sz, "<%ld.%d@%s>", (long)t, (int)(t % 9999), domain);
}

/*
 * fl_smtp_send — sendmail popen으로 텍스트 메일 발송
 */
FLValue fl_smtp_send(FLValue from_v, FLValue to_v,
                     FLValue subject_v, FLValue body_v) {
    const char* from    = smtp_cstr(from_v);
    const char* to      = smtp_cstr(to_v);
    const char* subject = smtp_cstr(subject_v);
    const char* body    = smtp_cstr(body_v);

    if (!from[0] || !to[0]) return fl_bool(0);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "/usr/sbin/sendmail -f '%s' -oi '%s'", from, to);

    FILE* f = popen(cmd, "w");
    if (!f) return fl_bool(0);

    char date[128], msgid[256];
    make_date_header(date, sizeof(date));
    make_msgid(msgid, sizeof(msgid), from);

    fprintf(f,
        "Date: %s\r\n"
        "From: %s\r\n"
        "To: %s\r\n"
        "Subject: %s\r\n"
        "Message-ID: %s\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Transfer-Encoding: 8bit\r\n"
        "\r\n"
        "%s\r\n",
        date, from, to, subject, msgid, body);

    int rc = pclose(f);
    return fl_bool(rc == 0);
}

/*
 * fl_smtp_send_html — HTML 멀티파트 메일 발송
 */
FLValue fl_smtp_send_html(FLValue from_v, FLValue to_v,
                          FLValue subject_v, FLValue html_v) {
    const char* from    = smtp_cstr(from_v);
    const char* to      = smtp_cstr(to_v);
    const char* subject = smtp_cstr(subject_v);
    const char* html    = smtp_cstr(html_v);

    if (!from[0] || !to[0]) return fl_bool(0);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "/usr/sbin/sendmail -f '%s' -oi '%s'", from, to);

    FILE* f = popen(cmd, "w");
    if (!f) return fl_bool(0);

    char date[128], msgid[256];
    make_date_header(date, sizeof(date));
    make_msgid(msgid, sizeof(msgid), from);

    const char* boundary = "fx_mail_boundary_2026";

    fprintf(f,
        "Date: %s\r\n"
        "From: %s\r\n"
        "To: %s\r\n"
        "Subject: %s\r\n"
        "Message-ID: %s\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: multipart/alternative; boundary=\"%s\"\r\n"
        "\r\n"
        "--%s\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Transfer-Encoding: 8bit\r\n"
        "\r\n"
        "%s\r\n"
        "\r\n"
        "--%s--\r\n",
        date, from, to, subject, msgid,
        boundary, boundary, html, boundary);

    int rc = pclose(f);
    return fl_bool(rc == 0);
}

/*
 * fl_smtp_test — 테스트 메일 발송
 */
FLValue fl_smtp_test(FLValue to_v) {
    const char* to = smtp_cstr(to_v);
    if (!to[0]) return fl_bool(0);

    FLValue from    = fl_str_val("fx-mail@localhost");
    FLValue to_val  = fl_str_val(to);
    FLValue subject = fl_str_val("[fx-mail] SMTP 테스트");
    FLValue body    = fl_str_val("fx-mail SMTP 테스트입니다.\nFreeLang fx C 네이티브에서 발송됨.");

    return fl_smtp_send(from, to_val, subject, body);
}
