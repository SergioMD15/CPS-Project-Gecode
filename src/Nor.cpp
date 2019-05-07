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
  IntVarArray result;
  VB inputs;

public:
  Nor(VI g, const int n, const int _depth) : num_inputs(n), depth(_depth), max_nodes(pow(2, depth + 1)-1), max_nors(pow(2, depth) - 1),
                      result(*this, 4 * max_nodes, -1, max_nodes), inputs((num_inputs + 1) * pow(2, num_inputs))
  {
    initializeInputs();
    // First value has to be 1
    rel(*this, result[0] == 1);
    for(int i = 4 * max_nors; i < 4 * max_nodes; ++i) {
      if((i - 1) % 4 == 0)
        rel(*this, result[i] > -1);
    }
    
    // If a node is not a gate then the subreee rooted at it is all 0
    for(int i = 1; i < 4*max_nodes; ++i) {
      if((i - 1) % 4 == 0){
        rel(*this, (result[i] != -1) >> ((result[i+1] == 0) and (result[i+2] == 0)));
        rel(*this, result[i+1] <= result[i+2]);
      }
    }

    for (int i = 1; i < max_nodes; ++i){

      // CONSTRAINTS FOR THE ELEMENTS
      rel(*this, result[i] < max_nodes and result[i] >= -1);
      if(i % 4 == 0){
        rel(*this, result[i] != result[i-4]);
        rel(*this, result[i] > 1);
      }
      if((i - 1) % 4 == 0){
        // The elements index has to be positive
        rel(*this, result[i] >= -1);
        // The elements indicating the type of the node has to be
        // either -1 or greater than 0
        rel(*this, (result[i+1] != result[i+2]
                and (result[i+2] > 1 and result[i+1] > 1 and result[i+1] < result[i+2]))
                or (result[i+1] == 0 and result[i+2] == 0));
      }
        // CONSTRAINTS FOR THE INPUTS
        // if(result[i].val() != -1){
          // // Inputs or zero values cannot have inputs
          // rel(*this, result[i+1] == 0 and result[i+2] == 0);

        // // CONSTRAINTS FOR THE NOR GATES
        // } else{
        //   // The NOR gates need inputs greater than 1
        //   rel(*this, result[i+1] > 1 and result[i+2] > 1);
        //   // The NOR gates' inputs need to be lower than max_nodes
        //   rel(*this, result[i+1] < max_nodes and result[i+2] < max_nodes);
        //   // NOR gate inputs' cannot be the same
        //   rel(*this, result[i+1] != result[i+2]);
        //   // NOR gate inputs' cannot be lower than NOR gate index
        //   rel(*this, result[i+1] > result[i-1] and result[i+2] > result[i-1]);
        //   // The first input of a NOR gate needs to be defined
        //   // immediately after the NOR gate.
        //   rel(*this, result[i+1] == result[i+3]);
        // }
    }

    // At most 2^(depth) - 1 NOR gates in the circuit.
    count(*this, result, -1, IRT_LQ, max_nors);

    // If the array size is greater than 1, every value greater than 1 needs
    // to appear twice in the solution array (once when it is used as input
    // and another one when it is defined).
    if(result.size() > 1){
      for(int i = 2; i < max_nodes + 1; ++i){
        if(i%4 == 0)
          count(*this, result, i, IRT_EQ, 1);
      }
    }

    branch(*this, result, INT_VAR_NONE(), INT_VAL_MIN());
  }

  int getNum_inputs(){
    return num_inputs;
  }

  int getDepth(){
    return depth;
  }

  VB getInputs(){
    return inputs;
  }

  IntVarArray getResult(){
    return result;
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

  void print(VI original)
  {
    // First print the input
    int size = count_gates();
    cout << num_inputs << endl;
    for(int v = 0; v < original.size(); ++v){
      cout << original[v] << endl;
    }
    // Then print depth and size
    cout << depth << " " << size << endl;
    // Then print solution
    int max_col = 4;
    for (int u = 0; u < result.size(); ++u){
      cout << result[u].val() << " ";
      if (((u + 1) % max_col == 0) and u != 0)
        cout << endl;
    }
    cout << endl;
  }
};

  VI norResult(Nor* solution, VI left, VI right){
    int size = solution->getNum_inputs();
    VI result_nor(pow(2, size));
    for(int i = 0; i < left.size(); i++){
      result_nor[i] = (!(left[i] or right[i]));
    }
    return result_nor;
  }

  /**
  * Given an index returns a list with the values corresponding
  * to that input.
  */
  VI getInputs(Nor* solution, int index){
    int num_inputs = solution -> getNum_inputs();
    int start = index * pow(2, num_inputs);
    int end = (index + 1) * pow(2, num_inputs);

    VB inputs = solution -> getInputs();
    VI copy(pow(2, num_inputs));
    for(int i = start; i < end; i++){
      copy[i] = inputs[i];
    }
    return copy;
  }

  int findIndex(IntVarArray list, int index){
    // We have the value for the result
    // and we want to find the index
    if(index == 0){
      return 0;
    }
    else{
      for(int i = 1; i < list.size(); i++){
        if((i - 1) % 4 == 0 && list[i].val() == index){
          return i;
        }
      }
    }
  }

  VI norOperation(Nor* solution, int index){
    IntVarArray list = solution -> getResult();
    if(solution->getDepth() == 0){
      return getInputs(solution, list[1].val());
    }

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
      left = getInputs(solution, index_first + 1);
    } else{
      left = norOperation(solution, index_first);
    }

    // Same as before with the right side.
    if(list[index_second + 1].val() != -1){
      right = getInputs(solution, index_first + 1);
    } else{
      right = norOperation(solution, index_first);
    }
    return norResult(solution, left, right);
  }

Nor* compare(Nor* first, Nor* second){
  if(first -> getDepth() > second -> getDepth()){
    return second;
  } else if (first->getDepth() < second->getDepth()){
    return first;
  } else{
    if(first->count_gates() > second->count_gates()){
      return second;
    } else if (first->count_gates() < second->count_gates()){
      return first;
    } else{
      return first;
    }
  }
}

int main(int argc, char *argv[])
{
  try
  {
    Nor* valid_solution;
    int n, u;
    cin >> n;
    VI g(pow(2, n));
    const int max_depth = 4;
    for (int k = 0; k < pow(2, n); ++k)
    {
      cin >> u;
      g[k] = u;
    }

    for(int j = 0; j < max_depth; j++){
      if(valid_solution){
        break;
      }
      cout << endl << "Looking for a solution with depth " << j << endl;
      Nor* mod = new Nor(g, n, j);
      BAB<Nor> e(mod);
      delete mod;
      Nor* s = e.next();
      while(s != NULL){
        VI result =  norOperation(s, 0);
        bool valid = true;
        for(int i = 0; i < result.size(); i++){
          if(result[i] != g[i]){
            valid = false;
            break;
          }
        }
        if(valid){
          cout << "SOLUTION FOUND" << endl;
          if(valid_solution)
            valid_solution = compare(s, valid_solution);
          else{
            valid_solution = s;
          }
          break;
        }
      }
    }
    valid_solution->print(g);
  }
  catch (Exception e)
  {
    cerr << "Gecode exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}
