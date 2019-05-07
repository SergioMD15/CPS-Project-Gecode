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

  int num_inputs;
  int depth;
  int max_nodes;
  int max_nors;


protected:
  // Type of each element starting from the bottom
  IntVarArray result;
  VB inputs;

public:
  Nor(const VI &g, const int n, const int _depth) : num_inputs(n), depth(_depth), max_nodes(pow(2, depth + 1) - 1)
                                            , max_nors(pow(2, depth) - 1), result(*this, 4*max_nodes, -1, max_nodes)
                                            , inputs((num_inputs + 1) * pow(2, num_inputs))
  {
    initializeInputs();
    // First value has to be 1
    rel(*this, result[0] == 1);
    
    // The leaves cannot be gates
    for(int i = 4*max_nors; i < max_nodes; ++i) {
      if((i - 1) % 4 == 0)
        rel(*this, result[i+1] != -1);
    }
    for (int i = 1; i < max_nodes; ++i){

      // CONSTRAINTS FOR THE ELEMENTS
      if((i - 1) % 4 == 0){
        // The elements index has to be positive
        rel(*this, result[i-1] > 0);
        // The elements indicating the type of the node has to be
        // either -1 or greater than 0
        rel(*this, result[i] == -1 or result[i] >= 0);
        
        // CONSTRAINTS FOR THE INPUTS
        if(result[i].val() > 0){
          // Inputs or zero values cannot have inputs
          rel(*this, result[i] > 0 and result[i+1] == 0 and result[i+2] == 0);

        // CONSTRAINTS FOR THE NOR GATES
        } else if(result[i].val() == -1){
          // The NOR gates need inputs greater than 1
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
    }

    // At most 2^(depth) - 1 NOR gates in the circuit.
      count(*this, result, -1, IRT_LQ, max_nors);

    // If the array size is greater than 1, every value greater than 1 needs
    // to appear twice in the solution array (once when it is used as input
    // and another one when it is defined).
    if(result.size() > 1){
      for(int i = 2; i < max_nodes + 1; ++i){
        count(*this, result, i, IRT_EQ, 2);
      }
    }
    // The total number of elements needs to be odd.
    // and also multiple of 4
    IntVar e = expr(*this, result.size());
    rel(*this, e % 4 == 0);
    rel(*this, (e / 4) % 2 != 0);


    // Main constraint to check the NOR behavior
    VI nor_res = norOperation(result, 0);
    for(int i = 0; i < g.size(); i++){
      IntVar res = expr(*this, nor_res[i]);
      IntVar original = expr(*this, g[i]);
      rel(*this, res == original);
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
      copy[i] = inputs[i];
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
  int value = 0;
  int size = (num_inputs + 1) * (pow(2,num_inputs));
  vector<int> inputs(size);
  int permutation = pow (2, num_inputs-1);

  // Initialize with zeros
  for (int i = 0; i < (2 ^ num_inputs) + 1; i++){
      inputs[i] = value;
  }
  for (int i = 2 ^ num_inputs + 1; i < size; i++){
    if (i % permutation == 0){
	    if(value == 0)
	        value = 1;
	    else
	        value = 0;
	  }
    if (i % int (pow (2, num_inputs)) == 0 and i != pow(2,num_inputs)){
	    permutation /= 2;
	  }
      inputs[i] = value;
    }
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
};

int main(int argc, char *argv[])
{
  try
  {
    string file;
    cin >> file;
    ifstream in(file);
    int n;
    in >> n;
    VI g(pow(2, n));
    int u;
    const int max_depth = 4;
    for (int k = 0; k < pow(2, n); ++k)
    {
      in >> u;
      g[k] = u;
    }
    for(int j = 0;j < max_depth; j ++){
      cout << 'Looking for solution with depth ' << j << endl;
      Nor* mod = new Nor(g, n, j);
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
  }
  catch (Exception e)
  {
    cerr << "Gecode exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}
