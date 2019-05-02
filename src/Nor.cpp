#include <fstream>
#include <vector>
#include <cmath>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace std;
using namespace Gecode;

typedef vector<int> VI;
typedef vector<bool> VB;

class Nor : public Space
{

private:
  // Number of inputs
  int n;
  int depth;
  int max_nodes;

protected:
  // Type of each element starting from the bottom
  IntVarArray result;
  VB inputs;

public:
  Nor(const VI &g) : n(g.at(0)), depth(4), max_nodes(pow(2, depth + 1)), result(*this, n, -1, max_nodes)
  {
    initializeInputs();
    // First value has to be 1
    rel(*this, result[0] == 1);
    for (int i = 1; i < max_nodes; ++i){

      // CONSTRAINTS FOR THE ELEMENTS
      if((i - 1) % 4 == 0){
        // The elements index has to be positive
        rel(*this, result[i-1] > 0);
        // The elements indicating the type of the node has to be
        // either -1 or greater than 0
        rel(*this, result[i] == -1 or result[i] > 0);
        
        // CONSTRAINTS FOR THE INPUTS
        if(result[i].val() >= 0){
          // Inputs or zero values cannot have inputs
          rel(*this, result[i+1] == 0 and result[i+2] == 0);

        // CONSTRAINTS FOR THE NOR GATES
        } else if(result[i].val() == -1){
          // The NOR gates need inputs
          rel(*this, result[i+1] > 1 and result[i+2] > 1);
          // The NOR gates' inputs need to be lower than max_nodes
          rel(*this, result[i+1] < max_nodes and result[i+2] < max_nodes);
          // NOR gate inputs' cannot be the same
          rel(*this, result[i+1] != result[i+2]);
          // NOR gate inputs' cannot be lower than NOR gate index
          rel(*this, result[i+1] < result[i-1] and result[i+2] < result[i-1]);
        }
      }
      // At most 2^(depth) - 1 NOR gates in the circuit.
      count(*this, result, -1, IRT_LQ, pow(2, depth) - 1);
      
      // If the array size is greater than 1, every value greater than 1 needs
      // to appear twice in the solution array (once when it is used as input
      // and another one when it is defined).
      if(result.size() > 1){
        vector<int> v(max_nodes);
        for(i = 0; i < max_nodes; ++i){
          v.push_back(i);
        }
        count(*this, result, v, IRT_GQ, 2);
      }

      // The final size of the array has to be odd.
      rel(*this, fmod(result.size(), 2));
    }
    branch(*this, result, INT_VAR_NONE(), INT_VAL_MIN());
  }

  // IntVarArray nor_result(BoolVarArray left, BoolVarArray right) const
  // {
  //   return q[i * n + j];
  // }

  int generator()
  { 
    static int i = 1;
    return i++;
  }

  void initializeInputs(){
    int value = 0;
    int permutation = pow(2, n-1);
    inputs.push_back(false);
		for(int i = 1; i < n * 2^n; i++){
      if(i % permutation == 0){
        value = swap(value);
      }
			if(i % (2^n) == 0){
        permutation /= 2;
			}
			inputs.push_back(value);
		}
  }

  int swap(int value){
			return value = 0 ? 1: 0;		
	}

  Nor(Nor &s) : Space(s)
  {
    n = s.n;
    depth = s.depth;
    max_nodes = s.max_nodes;
    inputs = s.inputs;
    result.update(*this, s.result);
  }

  virtual Space* copy()
  {
    return new Nor(*this);
  }

  int count_gates(){
    int counter = 0;
    for(int i = 0; i < result.size(); i++){
      if(result[i].val() == -1)
        counter++;
    }
    return counter;
  }

  void print(void)
  {
    int nor_gates = count_gates();
    int max_col = 4;
    for (int u = 0; u < n; ++u)
      if (max_col < result[u].val())
        max_col = result[u].val();

    cout << max_col << endl;

    for (int u = 0; u < n; ++u)
      cout << u + 1 << ' ' << result[u].val() << endl;
  }

  // virtual void constraint(const Space &_b)
  // {
  //   const Nor &b = static_cast<const Nor &>(_b);
  //   int max_col = -1;
  //   for (int u = 0; u < n; ++u)
  //     if (max_col < b.result[u].val())
  //       max_col = b.result[u].val();

  //   for (int u = 0; u < n; ++u)
  //     rel(*this, result[u] < max_col);
  // }
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
