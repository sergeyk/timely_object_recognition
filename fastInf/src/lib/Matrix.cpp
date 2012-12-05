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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <Matrix.h>

long double **ReadInputMatrix(FILE *in, int *rows, int *cols)
{
  char line[MAX_LINE_LEN];
  int i, j;
  long double **vec;
  char *word;
  long double tmp;
  
  *rows = *cols = 0;

  /* first pass on input file to figure out **vec dimensions */
  while(fgets(line, MAX_LINE_LEN, in)) {
    word = strtok(line, " ");
    if (*cols == 0) { 
      while (word && (isdigit(word[0]) || (word[0] == '-') || !strncmp(word, "NaN", 3))) {
	(*cols)++;
	word = strtok(NULL, " ");
      }
      if (*cols) (*rows)++;
    }
    else if (isdigit(word[0])  || (word[0] == '-') || !strncmp(word, "NaN", 3))(*rows)++;  
  }

  printf("Detected %d lines  with %d elements each\n", *rows, *cols);

  /* allocate vec */
  vec = NewMatrix(*rows, *cols);
    
  /* rewind file */
  fseek(in, 0, SEEK_SET); 

  /* read data into array */
  i = 0;
  while(fgets(line, 1000, in)) {
    j = 0;
    word = strtok(line, " ");
    while (word && (isdigit(word[0]) || (word[0] == '-') || !strncmp(word, "NaN", 3))) {
      if (!strcmp(word, "NaN"))
	tmp = MININF;
      else
	tmp = strtod(word, (char **)NULL);
      
      vec[i][j++] = tmp;
      /*printf ("%s = %.4f ",word, vec[i][j-1]);*/
      word = strtok(NULL, " ");
    }
    
    /* unless there are dim numbers in this line - discard it */
    if (j == *cols) {
      i++;
      /*printf ("\n");*/
    }
    else
      printf ("Discarded row %d: had only %d legit entries\n", i+1, j);
  }

  /* return actual number of lines read */
  *rows = i;
  printf("%d lines read with %d elements each\n", *rows, *cols);

  return(vec);
}

void WriteOutputMatrix(FILE *out, long double **matrix, int rows, int cols)
{
  register int i, j;

  printf("Writing to output file.\n");
  for (i = 0; i < rows; i++) {
    for(j = 0; j < cols; j++)
      fprintf (out, "%.3f  ", (double) matrix[i][j]); // change.imcgraw cast to long double to get rid of warningsmmc
     fprintf(out, "\n");
   }
 }

long double **CopyMatrix(long double **A, int rows, int cols)
{
  long double **B;
  int i, j;
  
  B = NewMatrix(rows, cols);
  
  for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
      B[i][j] = A[i][j];
  
  return B;
}

void DumpMatrix(FILE *f, long double **A, int rows, int cols)
{
  int i, j;
  
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++)
      fprintf(f, "%.4lf ", (double) A[i][j]);
    //change.imcgraw cast to long double to get rid of warnings
    fprintf(f, "\n");
  }
}

/****************************************************************************
 * Memory management
 ****************************************************************************/

long double **NewMatrix(int rows, int cols)
{
  int i;
  long double **matrix;
  
  if ( !(matrix = (long double **)calloc(rows , sizeof(long double *))) ) {
    fprintf(stderr, "malloc of %ld bytes failed at NewMatrix\n", (int)rows*sizeof(long double *));
    exit(-1);
  }
  
  for (i=0; i<rows; i++) 
    if ( !(matrix[i] = (long double *)calloc(cols , sizeof(long double))) ) {
      fprintf(stderr, "malloc of %ld bytes for row %d failed at NewMatrix\n",
	      (int)rows*sizeof(long double *), i);
      exit(-1);
    }
      
  
  return(matrix);
}

void FreeMatrix(long double **matrix, int rows)
{
  int i;

  for (i=0; i<rows; i++)
    free(matrix[i]);

  free(matrix);
}
