#include "sm4.h"

#include <QDebug>

SM4::SM4(uint32_t *key)
{
    for (int i = 0; i < 4; i++)
    {
        m_key[i] = key[0];
    }

    memset(CX, 0, sizeof(uint32_t) * 36);
    memset(MK, 0, sizeof(uint32_t) * 4);
    memset(rk, 0, sizeof(uint32_t) * 32);
    memset(CK, 0, sizeof(uint32_t) * 32);
    memset(ck, 0, sizeof(uint32_t) * 32);
}

uint32_t SM4::XunConvert(uint32_t x, uint8_t n)
{
    return ((x << n) | (x >> (32 - n)));
}

void SM4::ProductFk()
{
    FK[0] = 0xa3b1bac6;
    FK[1] = 0x56aa3350;
    FK[2] = 0x677d9197;
    FK[3] = 0xb27022dc;
}

void SM4::ProductCK()
{
    int i, j;
    uint8_t ckJ[4];
    for (i = 0; i < 32; i++)
    {
        for (j = 0; j < 4; j++)
        {
            ckJ[j] = ((4 * i + j) * 7) % 256;
        }
        CK[i] = (((ckJ[0] << 8) << 8) << 8) | ((ckJ[1] << 8) << 8) | (ckJ[2] << 8) | ckJ[3];
    }
}

uint32_t SM4::Sbox(uint32_t x)
{
    uint8_t *px = (uint8_t *)&x;
    px[0] = S[(px[0] >> 4) & 0x0f][px[0] & 0x0f];
    px[1] = S[(px[1] >> 4) & 0x0f][px[1] & 0x0f];
    px[2] = S[(px[2] >> 4) & 0x0f][px[2] & 0x0f];
    px[3] = S[(px[3] >> 4) & 0x0f][px[3] & 0x0f];
    return x;
}

uint32_t SM4::LDconvert(uint32_t x)
{
    return (x^XunConvert(x, 13) ^ XunConvert(x, 23));
}

uint32_t SM4::LEconvert(uint32_t x)
{
    return (x^XunConvert(x, 2) ^ XunConvert(x, 10) ^ XunConvert(x, 18) ^ XunConvert(x, 24));
}

uint32_t SM4::TDconvert(uint32_t x)
{
    return LDconvert(Sbox(x));
}

uint32_t SM4::TEconvert(uint32_t x)
{
    return LEconvert(Sbox(x));
}

void SM4::ProductRk()
{
    uint32_t K[36];

    int i;
    ProductFk();
    ProductCK();
    for (i = 0; i < 36; i++)
    {
        if (i < 4)
        {
            K[i] = MK[i] ^ FK[i];
        }
        else
        {
            rk[i - 4] = K[i] = K[i - 4] ^ TDconvert(K[i - 3] ^ K[i - 2] ^ K[i - 1] ^ CK[i - 4]);
        }
    }
}

void SM4::encode(uint32_t *plaintext, uint32_t *ciphertext, int len)
{
    int i, j, k;
    for (i = 0; i < 4; i++)
    {
        MK[i] = m_key[i];
    }
    ProductRk();
    for (k = 0; k <= len; k++)
    {
        if (k != 0 && k % 4 == 0)
        {
            for (i = 0, j = k - 4; i < 4; i++,j++)
            {
                CX[i] = plaintext[j];
            }

            for (i = 4; i < 36; i++)
            {
                CX[i] = CX[i - 4] ^ TEconvert(CX[i - 1] ^ CX[i - 2] ^ CX[i - 3] ^ rk[i - 4]);
            }
            for (i = k - 4, j = 35; j > 31; i++, j--)
            {
                ciphertext[i] = CX[j];
            }
        }
    }
}

void SM4::decode(uint32_t *ciphertext, uint32_t *plaintext, int len)
{
    int i, j, k;
    for (i = 0; i < 4; i++)
    {
        MK[i] = m_key[i];
    }
    ProductRk();
    for (k = 0; k <= len; k++)
    {
        if (k != 0 && k % 4 == 0)
        {
            for (i = 0, j = k - 4; i < 4; i++, j++)
            {
                CX[i] = ciphertext[j];
            }
            for (i = 4, j = 35; i < 36; i++, j--)
            {
                CX[i] = CX[i - 4] ^ TEconvert(CX[i - 1] ^ CX[i - 2] ^ CX[i - 3] ^ rk[j - 4]);
            }
            for (i = k-4, j = 35; j > 31; i++, j--)
            {
                plaintext[i] = CX[j];
            }
        }
    }
}

void SM4::convert_to_32(uint32_t *output, QByteArray input, uint32_t len)
{
    uint32_t i, j;

    for (i = 0, j = 0; j < len; j += 4, i++)
    {
        output[i] = ( input[j] & 0xFF) |
                    ((input[j + 1]  & 0xFF) << 8) |
                    ((input[j + 2]  & 0xFF) << 16) |
                    ((input[j + 3]  & 0xFF) << 24);
    }
}

QByteArray SM4::convert_to_8(uint32_t *input, uint32_t len)
{
    QByteArray output;
    output.resize(len * 4);

    for (uint32_t i = 0, j = 0; i < len; j += 4, i++)
    {
        output[j]     = (input[i] & 0x000000FF);
        output[j + 1] = (input[i] & 0x0000FF00) >> 8;
        output[j + 2] = (input[i] & 0x00FF0000) >> 16;
        output[j + 3] = (input[i] & 0xFF000000) >> 24;
    }

    return output;
}

QByteArray SM4::encrypt(QByteArray input)
{
    uint32_t input_len = input.length();
    uint8_t remain, remain2;
    uint32_t new_len, final_len;
    uint32_t *in_data32, *out_data32;
    QByteArray len_info, final_input;

    len_info.resize(4);
    len_info[0] = (input_len & 0x000000FF);
    len_info[1] = (input_len & 0x0000FF00) >> 8;
    len_info[2] = (input_len & 0x00FF0000) >> 16;
    len_info[3] = (input_len & 0xFF000000) >> 24;

    final_input = len_info + input;
    input_len += 4;

    remain  = input_len % 4;
    if (remain != 0)
    {
        new_len = input_len + 4 - remain;
        final_input.resize(new_len);
        for (uint8_t i = 0; i < (4 - remain); i++)
        {
            final_input[input_len + i] = 0x00;
        }
    }
    else
    {
        new_len = input_len;
    }

    remain2 = new_len % 16;
    if (remain2 != 0)
    {
        final_len = new_len + 16 - remain2;
        final_input.resize(final_len);
        for (uint8_t i = 0; i < (16 - remain2); i++)
        {
            final_input[new_len + i] = 0x00;
        }
    }
    else
    {
        final_len = new_len;
    }

    in_data32 = new uint32_t [final_len / 4];
    out_data32 = new uint32_t [final_len / 4];

    convert_to_32(in_data32, final_input, final_len);
    encode(in_data32, out_data32, final_len / 4);
    QByteArray output = convert_to_8(out_data32, final_len / 4);

    delete [] in_data32;
    delete [] out_data32;

    return output;
}

QByteArray SM4::decrypt(QByteArray input)
{
    uint32_t *in_data32, *out_data32;
    uint32_t input_len = input.length();
    uint32_t real_len = 0;
    QByteArray output, final_output;

    in_data32 = new uint32_t [input_len / 4];
    out_data32 = new uint32_t [input_len / 4];

    convert_to_32(in_data32, input, input_len);
    decode(in_data32, out_data32, input_len / 4);
    output = convert_to_8(out_data32, input_len / 4);

    real_len = (output[0] & 0xFF) |
            ((output[1] & 0xFF) << 8) |
            ((output[2] & 0xFF) << 16) |
            ((output[3] & 0xFF) << 24);
    output.resize(real_len + 4);
    final_output = output.right(real_len);

    delete [] in_data32;
    delete [] out_data32;

    return final_output;
}
