/* Copyright 2009 Ariel Jaimovich, Ofer Meshi, Ian McGraw and Gal Elidan */


/*
This file is part of FastInf library.

FastInf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

FastInf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with FastInf.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MATRIX_H
#define __MATRIX_H

#define MAX_LINE_LEN 1000
#define MININF      -10000

long double **ReadInputMatrix(FILE *in, int *rows, int *cols);
void     WriteOutputMatrix(FILE *out, long double **matrix, int rows, int cols);
long double **CopyMatrix(long double **A, int rows, int cols);
void     DumpMatrix(FILE *f, long double **A, int rows, int cols);
long double **NewMatrix(int rows, int cols);
void     FreeMatrix(long double **matrix, int rows);

#endif
