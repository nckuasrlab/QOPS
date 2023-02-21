__qpu__ void f(qreg q) {
  for(int i =0;i<10;i++){
    H(q[i]);
    Measure(q[i]);
  }
}

int main() {
  auto q = qalloc(10);
  f(q);
  q.print();
}