#include <fstream>
#include <vector>
#include <cmath>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace std;
using namespace Gecode;

typedef vector<int> VI;

class Nor : public Space
{

private:
  // Number of inputs
  int n;

protected:
  // Type of each element starting from the bottom
  IntVarArray result;
  BoolVarArray inputs;

public:
  Nor(const VI &g) : n(g.at(0)), result(*this, n, -1, pow(2,n)), inputs(*this, n*pow(2,n))
  {
    initializeInputs();
    // First we specify the values that are valid
    for (int i = 0; i < n; ++i){
      if(i % 3 == 0){
        if(result[i].val() != -1){
          rel(*this, result[i+1] == 0);
          rel(*this, result[i+2] == 0);
        } else if(result[i].val() == -1){
          rel(*this, result[i+1] > 1);
          rel(*this, result[i+2] > 1);
        }
      }
    }

    branch(*this, result, INT_VAR_NONE(), INT_VAL_MIN());
  }

  IntVarArray nor_result(BoolVarArray left, BoolVarArray right) const
  {
    return q[i * n + j];
  }

  void initializeInputs(){
    for (int i = 0; i < inputs.size(); i++){
      inputs[i]
    }
  }

  Nor(Nor &s) : Space(s)
  {
    n = s.n;
    c.update(*this, s.c);
  }

  virtual Space* copy()
  {
    return new Nor(*this);
  }

  void print(void) const
  {

    int max_col = -1;
    for (int u = 0; u < n; ++u)
      if (max_col < c[u].val())
        max_col = c[u].val();

    cout << max_col << endl;

    for (int u = 0; u < n; ++u)
      cout << u + 1 << ' ' << c[u].val() << endl;
  }

  virtual void constraint(const Space &_b)
  {
    const Nor &b = static_cast<const Nor &>(_b);
    int max_col = -1;
    for (int u = 0; u < n; ++u)
      if (max_col < b.c[u].val())
        max_col = b.c[u].val();

    for (int u = 0; u < n; ++u)
      rel(*this, c[u] < max_col);
  }
};

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
      return 1;
    ifstream in(argv[1]);
    int n, m;
    in >> n >> m;
    vector<int> g(n);
    for (int k = 0; k < m; ++k)
    {
      int u, v;
      in >> u >> v;
      --u;
      --v;
      g.push_back(v);
      g.push_back(u);
    }
    Nor* mod = new Nor(g);
    BAB<Nor> e(mod);
    delete mod;
    Nor* sant = e.next();
    Nor* s = e.next();
    while (s != NULL)
    {
      delete sant;
      sant = s;
      s = e.next();
    }
    sant->print();
    delete sant;
  }
  catch (Exception e)
  {
    cerr << "Gecode exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}
