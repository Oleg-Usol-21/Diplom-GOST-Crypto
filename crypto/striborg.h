/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#ifndef _GOST341112_H_
#define _GOST341112_H_

#include <cstdlib>
#include <cstring>

using namespace std;

class Streebog {
 private:
  int mode;
  void precalc_mul_table();
  void lps(unsigned char *in, unsigned long long *out);
  void ToHex(long long n, unsigned long long *c);
  void Xor64(unsigned long long *a, unsigned long long *b,
             unsigned long long *c);
  void Sum(unsigned char *a, unsigned char *b, unsigned char *c);
  void E(unsigned long long *k, unsigned long long *m);
  void g(unsigned char *h, unsigned char *m, unsigned long long *n);

 public:
  Streebog(int mode = 512);
  unsigned char *hash(unsigned char *message, unsigned int size);
  int getMode();
  void setMode(int mode);
};

#endif
