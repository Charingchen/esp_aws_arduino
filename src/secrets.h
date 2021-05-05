#include <pgmspace.h>

#define SECRET
#define THINGNAME "esp32-apha-1"

const char WIFI_SSID[] = "TELUS2433";
const char WIFI_PASSWORD[] = "h5ckbjbz8d";
const char AWS_IOT_ENDPOINT[] = "aewyz0z7rogih-ats.iot.us-west-2.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAJ0RHka/TZoRnQTVzz1vFAPMTDz0MA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTAzMjkyMjUw
MTBaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDAFR6vwV7BxTDppYMn
ayHbkUIPZ5zl1RVomsx3vHb6fyPo6IvrXmh/VxBwIleBLJFnZihep2kxyjybdrQi
r5hOPHM+FvjsPem2BphrXLjV5atD2Y30++l1/Gua76kv0Ah5o/BpdLeORmhj/CaC
oHBIuvIKcMNwFnIm2ID4OC3JCiJBGTrKFosbsSrbjqYyYIbHeiExYr/SDPwC1rpr
0UhEtP/SPla3EfftG2lH3dREeZB5jeUpn2w7SNmUD3otpef5VWzY4w+9kr/mOVap
AmjCXF5CUa6LfcUk0nvSx685CGzROfgBkTdCNvZJ9Y3k5lUISkUEt85gjL4o/gEL
LivdAgMBAAGjYDBeMB8GA1UdIwQYMBaAFELWQCUDBAUEoRmo/lslHQvakwkbMB0G
A1UdDgQWBBQC6+XHz+x7iPT3fzfhUlwrciJM7TAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAp63xoCka7YYckWxaMDE1Mb1u
MncMs0OQp3C+j9L4dnE0mOmSKwJwXGXLPCir1k3243i3Iw+xxJy/NYnco9oFIHbE
OuLWBsvv50/sQVvtYsTbJGCvruHVtYvyW4kAydnr74Fvo1YT4nrXVH+qchZGqnTz
XQG/fFCso6fy/cA1aEqAC3YQV7OwixTh2aALSlTAxUAqB23kcm0ODrkcj0+Pqon2
lRh5YzX5kZ5+bRyVsgqt+73ngIKMsBze7tTvvLT978t88pd441N1l8OKgs7t9PGM
MsHS8DVByIPvMfoEmhbwOB8HyJbtDbH+RXLFlwnSU7mWxpbQABKNZSdfxC/2Xw==
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAwBUer8FewcUw6aWDJ2sh25FCD2ec5dUVaJrMd7x2+n8j6OiL
615of1cQcCJXgSyRZ2YoXqdpMco8m3a0Iq+YTjxzPhb47D3ptgaYa1y41eWrQ9mN
9Pvpdfxrmu+pL9AIeaPwaXS3jkZoY/wmgqBwSLryCnDDcBZyJtiA+DgtyQoiQRk6
yhaLG7Eq246mMmCGx3ohMWK/0gz8Ata6a9FIRLT/0j5WtxH37RtpR93URHmQeY3l
KZ9sO0jZlA96LaXn+VVs2OMPvZK/5jlWqQJowlxeQlGui33FJNJ70sevOQhs0Tn4
AZE3Qjb2SfWN5OZVCEpFBLfOYIy+KP4BCy4r3QIDAQABAoIBAHv1pUYbbsFyNqyy
Sb/InaMLTSyP22tfZl+JdubW05m5Lz2BxbpxxNJZJ5yWLQ6J6pRmEytysqqVLyPN
KVcxJwNmc6DgRmh/VDJKc1bmEcrIWwyAe+JcKEAiVHc6r17RzIZjzCTiVAsXWQsd
kOTAEXZ+XvFp8FaHxcdX9XM7HiEcsIFKOB5gEO5uqU2sbqT/9UNG8NEqGIbrfXGj
fMDsRCylyfxJUEDkI2h6VrhiOHoBHVcUd2H9bRRHnP5EVn/EL53uLLG293ULGCQN
JHX5UZRBXGR8ZOBpkmVnPSGVmdpevVhUjms/vQItl01ezo/SSvwQwfgZ6SqXxF6j
zxx1MmkCgYEA6YbAI3FFe83jaziPFBeHE7fMTj6Qm6aJEkH2RfDz/koJIxtOcHqY
KnlLo4bM9Ag9QBULL0bnm9T/ZlBI9a1cd4RpnJNCCtY6cEs47AlV4UnOV8U5zjh4
fpv8Que0OcMGGL4GDfQMlQoaoHtaRhuZJBTYltE6qGxKNxnjVqYpTccCgYEA0pFX
bJeJDAUvEXUVNr8PYQEN1YOpFJujdVyvHysX8l1MB4IG9SADSRvXPAXYUydX9JTf
yefew4uINfZHUWb1IGaZ8E3vgXKLpOt175Ycm1bJRFNfRITCpxuxlk8dBpjJCKAm
hVJ4rwonV6pfmXvw0c9PXEncaaiRtJL4ioGoyTsCgYApSLs7CNFh27oywGL7CFXG
9J1Xz9O80Ko24hZpsvlPoUMQQ8mQNPwlth/bHv+ovLQnWhEKIJzdfcBRFCxx0l4g
evY4ygfI5qqaI5JCmDvogiXKeoGHpZAXwffM1C2WcJXpVd64JMb8qjae3G/PKR3f
tRe9Qev+znYoW5lHPkTbDQKBgGHNcDXYDKewoLQTFc9f3tzHiI1rodUYlUulyRFJ
Cs5f1ziFhIXGYMBMViRi8p9s3sWTgH5fXHCnFpU51tZQY/qwrT+Yc25ErefkFds6
df8QyFsb/lvs7hyAGHQgOvMddAK1oC68p6FlrfBiChVYI/rPt6S+CcppqYOd4CM1
JhiRAoGAOvThSvpk0miUGN8i9OVssJ/GJUlFOraHQSCLDgYRzIGolrGrNw0BPXg1
QT99Hy4Xzmt3OdpxwXy+rwUt1zHwEyUV5gxBd4hssHXdyrHlKm6OhbM5VbibExKX
XOeUof2QedwIzXapK8eRAb8MkIfHqej5+hPD00H33Gr9NGNLGQU=
-----END RSA PRIVATE KEY-----
)KEY";