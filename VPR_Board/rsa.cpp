#include "rsa.h"
#include "time.h"

RSA::RSA(QObject *parent) : QObject(parent)
{
    for(int i = 0;i < KEYMAX; i++)
    {
         this->rsa_m[i] = 0;
         this->rsa_p[i] = 0;
         this->rsa_q[i] = 0;
         this->rsa_n[i] = 0;
         this->rsa_d[i] = 0;
         this->rsa_e[i] = 0;
    }

    create_rsa_key();
}

QString RSA::rsa_decrypt(QString p_ciphertext)
{
    char plaintext_char[1024], ch;
    int i = 0, j = 3, k, c, temp;
    int count = 0, count2 = 0;
    struct slink *h,*p1,*p2;

    h = p1 = p2 = (struct slink * )malloc(SLINKLEN);
    QByteArray ciphertext_byte = p_ciphertext.toLatin1();

    for(int m = 0; m < ciphertext_byte.length(); m++)
    {
        c = ciphertext_byte.at(m);
        if(j == 3)
        {
            p1->bignum[KEYMAX - 2] = c;
            j--;
        }
        else if(j == 2)
        {
            temp = c - 48;
            j--;
        }
        else if(j == 1)
        {
            p1->bignum[KEYMAX - 1] = temp * 10 + c - 48;
            j--;
        }
        else if(j == 0)
        {
            p1->bignum[i] = c - 48;
            i++;
            if(i == p1->bignum[KEYMAX - 1])
            {
                i = 0;
                j = 3;
                count++;
                if (count == 1)
                {
                    h = p1;
                }
                else
                {
                    p2->next = p1;
                }
                p2 = p1;
                p1 = (struct slink * )malloc(SLINKLEN);
            }
        }
    }
    p2->next = NULL;

    p2 = (struct slink * )malloc(SLINKLEN);
    p1 = h;
    k = 0;
    if(h)
    {
        do
        {
            for(i = 0; i < KEYMAX; i++)
            {
                p2->bignum[i] = 0;
            }
            expmod(p1->bignum, rsa_d, rsa_n, p2->bignum);
            temp = p2->bignum[0] + p2->bignum[1] * 10 + p2->bignum[2] * 100;
            if (p2->bignum[KEYMAX - 2] == '0')
            {
                temp = 0 - temp;
            }
            ch = temp;
            plaintext_char[count2++] = ch;
            k++;
            p1 = p1->next;
            p2 = (struct slink * )malloc(SLINKLEN);
        }while (p1);
    }

    plaintext_char[count2++] = '\0';
    QString plaintext(plaintext_char);
    return plaintext;
}

QByteArray RSA::get_e_byte()
{
    QByteArray e_byte;
    e_byte.resize(rsa_e[99]);
    for (int i = 0; i < rsa_e[99]; i++)
    {
        e_byte[i] = rsa_e[i];
    }
    return e_byte;
}

QByteArray RSA::get_n_byte()
{
    QByteArray n_byte;
    n_byte.resize(rsa_n[99]);
    for (int i = 0; i < rsa_n[99]; i++)
    {
        n_byte[i] = rsa_n[i];
    }
    return n_byte;
}

void RSA::create_rsa_key()
{
    prime_random(rsa_p, rsa_q);
    mul(rsa_p, rsa_q, rsa_n);
    mov(rsa_p, rsa_p1);
    rsa_p1[0]--;
    mov(rsa_q, rsa_q1);
    rsa_q1[0]--;
    mul(rsa_p1, rsa_q1, rsa_m);
    erand(rsa_e, rsa_m);
    rsad(rsa_e, rsa_m, rsa_d);
}

int RSA::cmp(int a1[KEYMAX], int a2[KEYMAX])
{
    int l1, l2;
    l1 = a1[99];
    l2 = a2[99];

    if (l1 > l2)
    {
        return 1;
    }
    if (l1 < l2)
    {
       return -1;
    }
    for(int i = l1 - 1; i >= 0; i--)
    {
        if (a1[i] > a2[i])
        {
            return 1;
        }
        if (a1[i] < a2[i])
        {
            return -1;
        }
    }
    return 0;
}

void RSA::mov(int a[KEYMAX], int *b)
{
    for(int j = 0; j < KEYMAX; j++)
    {
        b[j] = a[j];
    }
}

void RSA::mul(int a1[KEYMAX], int a2[KEYMAX], int *c)
{
    int y, x, z, w;
    int l1, l2;

    l1 = a1[KEYMAX - 1];
    l2 = a2[KEYMAX - 1];

    if ((a1[KEYMAX - 2] == '-') && (a2[KEYMAX - 2] == '-'))
    {
        c[KEYMAX - 2] = 0;
    }
    else if (a1[KEYMAX - 2] == '-')
    {
        c[KEYMAX - 2] = '-';
    }
    else if (a2[KEYMAX - 2] == '-')
    {
        c[KEYMAX - 2] = '-';
    }

    for(int i = 0; i < l1; i++)
    {
        for(int j = 0; j < l2; j++)
        {
            x = a1[i] * a2[j];
            y = x / 10;
            z = x % 10;
            w = i + j;
            c[w] = c[w] + z;
            c[w + 1] = c[w + 1] + y + c[w] / 10;
            c[w] = c[w] % 10;
        }
    }
    w = l1 + l2;
    if(c[w - 1] == 0)
    {
        w = w - 1;
    }
    c[KEYMAX - 1] = w;
}

void RSA::add(int a1[KEYMAX], int a2[KEYMAX], int *c)
{
    int l1,l2;
    int len, temp[KEYMAX];
    int k = 0;

    l1 = a1[KEYMAX - 1];
    l2 = a2[KEYMAX - 1];
    if((a1[KEYMAX - 2] == '-') && (a2[KEYMAX - 2] == '-'))
    {
        c[KEYMAX - 2] = '-';
    }
    else if (a1[KEYMAX - 2] == '-')
    {
        mov(a1, temp);
        temp[KEYMAX - 2] = 0;
        sub(a2, temp, c);
        return;
    }
    else if (a2[KEYMAX-2]=='-')
    {
        mov(a2, temp);
        temp[98] = 0;
        sub(a1, temp, c);
        return;
    }

    if(l1 < l2)
    {
        len = l1;
    }
    else
    {
        len = l2;
    }
    for(int i = 0; i < len; i++)
    {
        c[i] = (a1[i] + a2[i] + k) % 10;
        k = (a1[i] + a2[i] + k) / 10;
    }
    if(l1 > len)
    {
        for(int i = len; i < l1; i++)
        {
            c[i] = (a1[i] + k) % 10;
            k = (a1[i] + k) / 10;
        }
        if(k != 0)
        {
            c[l1] = k;
            len = l1 + 1;
        }
        else
        {
            len = l1;
        }
    }
    else
    {
        for(int i = len; i < l2; i++)
        {
            c[i] = (a2[i] + k) % 10;
            k = (a2[i] + k) / 10;
        }
        if(k != 0)
        {
            c[l2] = k;
            len = l2 + 1;
        }
        else
        {
            len = l2;
        }
    }
    c[99] = len;
}


void RSA::sub(int a1[KEYMAX],int a2[KEYMAX],int *c)
{
    int l1, l2, i;
    int len, t1[KEYMAX], t2[KEYMAX];
    int k = 0;

    l1 = a1[KEYMAX - 1];
    l2 = a2[KEYMAX - 1];
    if ((a1[KEYMAX - 2] == '-') && (a2[KEYMAX - 2] == '-'))
    {
        mov(a1, t1);
        mov(a2, t2);
        t1[KEYMAX - 2] = 0;
        t2[KEYMAX - 2] = 0;
        sub(t2, t1, c);
        return;
    }
    else if(a2[KEYMAX - 2] == '-')
    {
        mov(a2, t2);
        t2[KEYMAX - 2] = 0;
        add(a1, t2, c);
        return;
    }
    else if (a1[KEYMAX - 2] == '-')
    {
        mov(a2, t2);
        t2[KEYMAX - 2] = '-';
        add(a1, t2, c);
        return;
    }

    if(cmp(a1, a2) == 1)
    {
        len = l2;
        for(i = 0; i < len; i++)
        {
            if ((a1[i] - k - a2[i]) < 0)
            {
                c[i] = (a1[i] - a2[i] - k + 10) % 10;
                k = 1;
            }
            else
            {
                c[i] = (a1[i] - a2[i] - k) % 10;
                k = 0;
            }
        }

        for(i = len; i < l1; i++)
        {
            if ((a1[i] - k )< 0)
            {
                c[i] = (a1[i] - k + 10) % 10;
                k = 1;
            }
            else
            {
                c[i] = (a1[i] - k) % 10;
                k = 0;
            }
        }
        if(c[l1 - 1] == 0)
        {
            len = l1 - 1;
            i = 2;
            while(c[l1 - i] == 0)
            {
                len = l1 - i;
                i++;
            }
        }
        else
        {
            len = l1;
        }
    }
    else if(cmp(a1, a2) == -1)
    {
        c[KEYMAX - 2] = '-';
        len = l1;
        for(i = 0; i < len; i++)
        {
            if ((a2[i] - k - a1[i]) < 0)
            {
                c[i] = (a2[i] - a1[i] - k + 10) % 10;
                k = 1;
            }
            else
            {
                c[i] = (a2[i] - a1[i] - k) % 10;
                k = 0;
            }
        }
        for(i = len; i < l2; i++)
        {
            if ((a2[i] - k) < 0)
            {
                c[i] = (a2[i] - k + 10) % 10;
                k = 1;
            }
            else
            {
                c[i] = (a2[i] - k) % 10;
                k = 0;
            }
        }
        if(c[l2 - 1] == 0)
        {
            len = l2 - 1;
            i = 2;
            while(c[l1 - i] == 0)
            {
                len = l1 - i;
                i++;
            }
        }
        else
        {
            len = l2;
        }
    }
    else if(cmp(a1, a2) == 0)
    {
        len = 1;
        c[len - 1] = 0;
    }
    c[KEYMAX - 1] = len;
}

void RSA::mod(int a[KEYMAX], int b[KEYMAX], int *c)
{
    int d[KEYMAX];
    mov(a, d);
    while(cmp(d, b) != -1)
    {
        sub(d, b, c);
        mov(c, d);
    }
}

void RSA::divt(int t[KEYMAX], int b[KEYMAX], int *c, int *w)
{
    int a1, b1, m;
    int d[KEYMAX], e[KEYMAX], f[KEYMAX], g[KEYMAX], a[KEYMAX];

    mov(t, a);
    for(int i = 0; i < KEYMAX; i++)
    {
       e[i] = 0;
       d[i] = 0;
       g[i] = 0;
    }
    a1 = a[KEYMAX - 1];
    b1 = b[KEYMAX - 1];
    if(cmp(a, b) == -1)
    {
        c[0] = 0;
        c[KEYMAX - 1] = 1;
        mov(t, w);
        return;
    }
    else if(cmp(a, b) == 0)
    {
        c[0] = 1;
        c[KEYMAX - 1] = 1;
        w[0] = 0;
        w[KEYMAX - 1] = 1;
        return;
    }

    m = a1 - b1;
    for(int i = m; i >= 0; i--)
    {
        for(int j = 0; j < KEYMAX; j++)
        {
            d[j] = 0;
        }
        d[i] = 1;
        d[KEYMAX - 1] = i + 1;
        mov(b, g);
        mul(g, d, e);

        while(cmp(a, e) != -1)
        {
            c[i]++;
            sub(a, e, f);
            mov(f, a);
        }
        for(int j = i; j < KEYMAX; j++)
        {
            e[j] = 0;
        }
    }
    mov(a, w);
    if (c[m] == 0)
    {
        c[KEYMAX - 1] = m;
    }
    else
    {
        c[KEYMAX - 1] = m + 1;
    }
}

void RSA::mulmod(int a[KEYMAX], int b[KEYMAX], int n[KEYMAX], int *m)
{
    int c[KEYMAX], d[KEYMAX];

    for(int i = 0; i < KEYMAX; i++)
    {
        d[i] = 0;
        c[i] = 0;
    }
    mul(a, b, c);
    divt(c, n, d, m);
}

void RSA::expmod(int a[KEYMAX], int p[KEYMAX], int n[KEYMAX], int *m)
{
    int t[KEYMAX], l[KEYMAX], temp[KEYMAX];
    int w[KEYMAX], s[KEYMAX], c[KEYMAX], b[KEYMAX];

    for(int i = 0; i < (KEYMAX - 1); i++)
    {
        b[i] = 0;
        l[i] = 0;
        t[i] = 0;
        w[i] = 0;
    }
    t[0] = 2;
    t[KEYMAX - 1] = 1;
    l[0] = 1;
    l[KEYMAX - 1] = 1;

    mov(l, temp);
    mov(a, m);
    mov(p, b);

    while(cmp(b, l) != 0)
    {
        for(int i = 0; i < KEYMAX; i++)
        {
            w[i] = 0;
            c[i] = 0;
        }
        divt(b, t, w, c);
        mov(w, b);

        if(cmp(c, l) == 0)
        {
            for(int i = 0; i < KEYMAX; i++)
            {
                w[i] = 0;
            }
            mul(temp, m, w);
            mov(w, temp);

            for(int i = 0; i < KEYMAX; i++)
            {
                w[i] = 0;
                c[i] = 0;
            }
            divt(temp, n, w, c);
            mov(c, temp);
        }

        for(int i = 0; i < KEYMAX; i++)
        {
            s[i] = 0;
        }
        mul(m, m, s);

        for(int i = 0; i < KEYMAX; i++)
        {
            c[i] = 0;
        }
        divt(s, n, w, c);
        mov(c, m);
    }

    for(int i = 0; i < KEYMAX; i++)
    {
        s[i] = 0;
    }
    mul(m, temp, s);

    for(int i = 0; i < KEYMAX; i++)
    {
        c[i] = 0;
    }
    divt(s, n, w, c);
    mov(c, m);
    m[KEYMAX - 2] = a[KEYMAX - 2];
}

bool RSA::is_prime_san(int p[KEYMAX])
{
    int a[KEYMAX], t[KEYMAX], s[KEYMAX], o[KEYMAX];
    for(int i = 0; i < KEYMAX; i++)
    {
        s[i] = 0;
        o[i] = 0;
        a[i] = 0;
        t[i] = 0;
    }
    t[0] = 1;
    t[KEYMAX - 1] = 1;
    a[0] = 2;
    a[KEYMAX - 1] = 1;

    sub(p, t, s);

    expmod(a, s, p ,o);
    if (cmp(o, t) != 0)
    {
        return false;
    }
    a[0] = 3;
    for(int i = 0; i < KEYMAX; i++)
    {
        o[i] = 0;
    }

    expmod(a, s, p ,o);
    if (cmp(o, t) != 0)
    {
        return false;
    }
    a[0] = 5;
    for(int i = 0; i < KEYMAX; i++)
    {
        o[i] = 0;
    }

    expmod(a, s, p ,o);
    if (cmp(o, t) != 0)
    {
        return false;
    }
    a[0] = 7;
    for(int i = 0;i < KEYMAX; i++)
    {
        o[i] = 0;
    }

    expmod(a, s, p ,o);
    if (cmp(o, t) != 0)
    {
        return false;
    }
    return true;
}

bool RSA::coprime(int e[KEYMAX], int s[KEYMAX])
{
    int a[KEYMAX], b[KEYMAX], c[KEYMAX], d[KEYMAX], o[KEYMAX], l[KEYMAX];

    for(int i = 0; i < KEYMAX; i++)
    {
        l[i] = 0;
        o[i] = 0;
        c[i] = 0;
        d[i] = 0;
    }
    o[0] = 0;
    o[KEYMAX - 1] = 1;
    l[0] = 1;
    l[KEYMAX - 1] = 1;
    mov(e, b);
    mov(s, a);

    do {
        if(cmp(b, l) == 0)
        {
            return true;
        }
        for(int i = 0; i < KEYMAX; i++)
        {
            c[i] = 0;
        }
        divt(a, b, d, c);
        mov(b, a);
        mov(c, b);
    } while(cmp(c, o) != 0);

    return false;
}

void RSA::prime_random(int *p, int *q)
{
    int k;
    time_t t;

    p[0] = 1;
    q[0] = 3;
    p[KEYMAX - 1] = 10;
    q[KEYMAX - 1] = 11;

    do {
        t = time(NULL);
        srand((unsigned long)t);
        for(int i = 1; i < (p[KEYMAX - 1] - 1); i++)
        {
            k = rand() % 10;
            p[i] = k;
        }
        k = rand() % 10;
        while (k == 0)
        {
            k = rand() % 10;
        }
        p[p[KEYMAX - 1] - 1] = k;
    } while(is_prime_san(p) != 1);

    do {
        t = time(NULL);
        srand((unsigned long)t);
        for(int i = 1; i < q[KEYMAX - 1]; i++)
        {
            k = rand() % 10;
            q[i] = k;
        }
    } while(is_prime_san(q) != 1);
}

void RSA::erand(int e[KEYMAX], int m[KEYMAX])
{
    int k;
    time_t t;
    e[KEYMAX - 1] = 5;

    do {
        t = time(NULL);
        srand((unsigned long)t);
        for(int i = 0; i < e[KEYMAX - 1] - 1; i++)
        {
            k = rand() % 10;
            e[i] = k;
        }
        while((k = rand() % 10) == 0)
        {
            k = rand() % 10;
        }
        e[e[KEYMAX - 1] - 1] = k;
    } while(coprime(e, m) != 1);
}

void RSA::rsad(int e[KEYMAX], int g[KEYMAX], int *d)
{
    int r[KEYMAX],n1[KEYMAX],n2[KEYMAX],k[KEYMAX],w[KEYMAX];
    int t[KEYMAX],b1[KEYMAX],b2[KEYMAX],temp[KEYMAX];

    mov(g, n1);
    mov(e, n2);
    for(int i = 0; i < KEYMAX; i++)
    {
        k[i] = 0;
        w[i] = 0;
        r[i] = 0;
        temp[i] = 0;
        b1[i] = 0;
        b2[i] = 0;
        t[i] = 0;
    }
    b1[KEYMAX - 1] = 0;
    b1[0] = 0;
    b2[KEYMAX - 1] = 1;
    b2[0] = 1;
    while(1)
    {
        for(int i = 0; i < KEYMAX; i++)
        {
            k[i] = 0;
            w[i] = 0;
        }
        divt(n1, n2, k, w);
        for(int i = 0; i < KEYMAX; i++)
        {
            temp[i] = 0;
        }
        mul(k, n2, temp);
        for(int i = 0; i < KEYMAX; i++)
        {
            r[i] = 0;
        }
        sub(n1, temp, r);

        if((r[KEYMAX - 1] == 1) && (r[0] == 0))
        {
            break;
        }
        else
        {
            mov(n2, n1);
            mov(r, n2);
            mov(b2, t);
            for(int i = 0; i < KEYMAX; i++)
            {
                temp[i] = 0;
            }
            mul(k, b2, temp);
            for(int i = 0; i < KEYMAX; i++)
            {
                b2[i] = 0;
            }
            sub(b1, temp, b2);
            mov(t, b1);
        }
    }

    for(int i = 0; i < KEYMAX; i++)
    {
        t[i] = 0;
    }
    add(b2, g, t);
    for(int i = 0;i < KEYMAX; i++)
    {
        temp[i] = 0;
        d[i] = 0;
    }
    divt(t, g, temp, d);
}
