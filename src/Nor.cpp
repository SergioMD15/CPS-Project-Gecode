#include <iostream>
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
  int num_inputs;
  int depth;
  int max_nodes;

protected:
  // Type of each element starting from the bottom
  IntVarArray result;
  VB inputs;

public:
  Nor(const VI &g) : num_inputs(g[0]), depth(4), max_nodes(pow(2, depth + 1)), 
                      result(*this, num_inputs, -1, max_nodes), inputs(num_inputs + 1 * 2^num_inputs)
  {
    initializeInputs();
    // // First value has to be 1
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
          // The first input of a NOR gate needs to be defined
          // immediately after the NOR gate.
          rel(*this, result[i+1] == result[i+3]);
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
          v[i] = i;
        }
        count(*this, result, v, IRT_EQ, 2);
      }

    }
    // The total number of elements needs to be odd.
    // and also multiple of 4
    rel(*this, result.size() % 4 == 0);
    rel(*this, (result.size() / 4) % 2 != 0);

    VI nor_res = norOperation(result, 0);
    for(int i = 0; i < g.size(); i++){
      rel(*this, nor_res[i] == g[i]);
    }

    branch(*this, result, INT_VAR_NONE(), INT_VAL_MIN());
  }

  VI norResult(VI left, VI right){
    VI result;
    for(int i = 0; i < left.size(); i++){
      result[i] = (!(left[i] or right[i]));
    }
    return result;
  }

  VI norOperation(IntVarArray list, int index){
    // We find the position of the NOR gate inputs'
    // identifiers (1 -1 2 3  --> Looking where are 2
    // and 3 in the array).
    int index_first = findIndex(list, index + 2);
    int index_second = findIndex(list, index + 3);
    VI left, right;
    
    // Get the inputs for the left part. If it
    // is a NOR gate, we iterate again until
    // getting a valid input.
    if(list[index_first + 1].val() != -1){
      left = getInputs(index_first + 1);
    } else{
      left = norOperation(list, index_first);
    }

    // Same as before with the right side.
    if(list[index_second + 1].val() != -1){
      right = getInputs(index_first + 1);
    } else{
      right = norOperation(list, index_first);
    }
    return norResult(left, right);
  }

  /**
  * Given an index returns a list with the values corresponding
  * to that input.
  */
  VI getInputs(int index){
    VI copy(pow(2, num_inputs));
    for(int i = index * pow(2, num_inputs); i < (index + 1) * pow(2, num_inputs); i++){
      copy.push_back(inputs[i]);
    }
    return copy;
  }

  int findIndex(IntVarArray list, int index){
    // We have the value for the result
    // and we want to find the index
    for(int i = 1; i < list.size(); i++){
      if((i - 1) % 4 == 0 && list[i].val() == index){
        return i;
      }
    }
  }

  void initializeInputs(){
    bool value = false;
    int permutation = pow(2, num_inputs - 1);

    // Initialize with zeros
    for(int i = 0; i < 2^num_inputs+1; i++){
      inputs.push_back(value);
    }
		for(int i = 2^num_inputs + 1; i < num_inputs * 2^num_inputs; i++){
      if(i % permutation == 0){
        value = swap(value);
      }
			if(i % int(pow(2, num_inputs)) == 0){
        permutation /= 2;
			}
			inputs.push_back(value);
		}
  }

  bool swap(int value){
			return value = false ? true: false;		
	}

  int count_gates(){
    int counter = 0;
    for(int i = 0; i < result.size(); i++){
      if(result[i].val() == -1)
        counter++;
    }
    return counter;
  }

  Nor(Nor &s) : Space(s)
  {
    num_inputs = s.num_inputs;
    depth = s.depth;
    max_nodes = s.max_nodes;
    inputs = s.inputs;
    result.update(*this, s.result);
  }

  virtual Space* copy()
  {
    return new Nor(*this);
  }

  void print(void)
  {
    int nor_gates = count_gates();
    int max_col = 4;
    for (int u = 0; u < result.size(); ++u){
      cout << result[u].val();
      if (u % max_col == 0)
        cout << endl;
    }
  }

  // virtual void constraint(const Space &_b)
  // {
  //   const Nor &b = static_cast<const Nor &>(_b);
  //   int max_col = -1;
  //   for (int u = 0; u < num_inputs; ++u)
  //     if (max_col < b.result[u].val())
  //       max_col = b.result[u].val();

  //   for (int u = 0; u < num_inputs; ++u)
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
    int n;
    in >> n;
    VI g(pow(2, n));
    g[0] = n;
    cout << g[0];
    int u;
    for (int k = 0; k < pow(2, n); ++k)
    {
      in >> u;
      g.push_back(u);
    }
    Nor* mod = new Nor(g);
    BAB<Nor> e(mod);
    delete mod;
    Nor* s = e.next();
    while (s != NULL)
    {
      s->print();
      s = e.next();
    }
    delete s;
  }
  catch (Exception e)
  {
    cerr << "Gecode exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}
