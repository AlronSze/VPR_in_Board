#ifndef RSA_H
#define RSA_H

#include <QObject>

#define KEYMAX   100
#define SLINKLEN sizeof(struct slink)

struct slink
{
    int bignum[KEYMAX];
    struct slink *next;
};

class RSA : public QObject
{
    Q_OBJECT
public:
    explicit RSA(QObject *parent = 0);

signals:

public slots:

private:
    int cmp(int a1[KEYMAX], int a2[KEYMAX]);
    void mov(int a[KEYMAX], int *b);
    void mul(int a1[KEYMAX], int a2[KEYMAX], int *c);
    void add(int a1[KEYMAX], int a2[KEYMAX], int *c);
    void sub(int a1[KEYMAX], int a2[KEYMAX], int *c);
    void mod(int a[KEYMAX], int b[KEYMAX], int *c);
    void divt(int t[KEYMAX], int b[KEYMAX], int *c, int *w);
    void mulmod(int a[KEYMAX], int b[KEYMAX], int n[KEYMAX], int *m);
    void expmod(int a[KEYMAX], int p[KEYMAX], int n[KEYMAX], int *m);
    bool is_prime_san(int p[KEYMAX]);
    bool coprime(int e[KEYMAX], int s[KEYMAX]);
    void prime_random(int *p,int *q);
    void erand(int e[KEYMAX], int m[KEYMAX]);
    void rsad(int e[KEYMAX], int g[KEYMAX], int *d);
    void create_rsa_key();

    int rsa_p[KEYMAX], rsa_q[KEYMAX], rsa_n[KEYMAX], rsa_d[KEYMAX];
    int rsa_e[KEYMAX], rsa_m[KEYMAX], rsa_p1[KEYMAX], rsa_q1[KEYMAX];

public:
    QString rsa_decrypt(QString p_ciphertext);
    QByteArray get_e_byte();
    QByteArray get_n_byte();
};

#endif // RSA_H
