
void compute_ref (double **a, double **b, double **c, double **d, int N, int num_threads) {

  for (int i = 0; i < N; i++){
    for(int j = 0; j < N; j++) {
      a[i][j] = 2 * c[i][j];
    }
  }


  for (int i = 0; i < N; i++) {
    for (int j = 0 ; j < N; j++) {
      d[i][j] = a[i][j] * b[i][j] ;
    }
  }



  for (int j = 0; j < N-1; j++) {
    for (int i = 0; i < N; i++) {
      d[i][j] = d[i][j+1] + c[i][j]; 
    }
  }


}
