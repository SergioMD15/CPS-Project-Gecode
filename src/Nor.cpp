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
    // Last elements of the tree (leafs cannot be NOR gates)
    for(int i = 4 * max_nors; i < 4 * max_nodes; ++i) {
      if((i - 1) % 4 == 0){
        rel(*this, result[i] > -1);
      }
    }
    
    // // If a node is not a gate then the subreee rooted at it is all 0
    for(int i = 1; i < 4 * max_nodes; ++i) {
      if((i - 1) % 4 == 0){
        // The type of the node needs to be lower than the number of inputs
        rel(*this, result[i] <= num_inputs);
        // Index values must be positive
        rel(*this, result[i-1] > 0);
        // Inputs are followed by zeros
        rel(*this, (result[i] != -1) >> ((result[i+1] == 0) and (result[i+2] == 0)));
        // NOR gates are followed by inputs greater than 1
        rel(*this, (result[i] == -1) >> ((result[i+1] > 1) and (result[i+2] > 1)));
        // NOR gates' left input is smaller than right input.
        rel(*this, (result[i] == -1) >> ((result[i+1] < result[i+2])));

        // Checking up to the last element
        if(i != (4 * max_nodes - 3)){
          // If we have a NOR gate, the left input would be defined immediately after the gate
          rel(*this, (result[i] == -1) >> ((result[i+1] == result[i+3])));
          // If we have a NOR gate, the left input has to be greater than the index of the NOR gate
          rel(*this, (result[i] == -1) >> ((result[i+1] > result[i-1])));
          // If we have two consecutive NOR gates, the second door inputs' have to be greater than
          // the ones from the first.
          rel(*this, (result[i] == -1 and result[i+4] == -1) >> ((result[i+2] < result[i+5])));
          // The index of the door has to be different from the index of the next door.
          rel(*this, result[i-1] != result[i+3]);
        }
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

  virtual void constrain(const Space& _b) {
    const Nor& nor = static_cast <const Nor&>( _b );
  }

  Nor(Nor &s) : Space(s)
  {
    num_inputs = s.num_inputs;
    depth = s.depth;
    max_nodes = s.max_nodes;
    max_nors = s.max_nors;
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
    VI copy(pow(2, num_inputs));
    if(index == -1){
      copy[0] = -1;
      return copy;
    }
    int start = index * pow(2, num_inputs);
    int end = (index + 1) * pow(2, num_inputs);

    VB inputs = solution -> getInputs();
    for(int i = start; i < end; i++){
      copy[i] = inputs[i];
    }
    return copy;
  }

  int findIndex(IntVarArray list, int value){
    // We have the value for the result
    // and we want to find the index
    if(value == 0){
      return 0;
    }
    else{
      for(int i = 1; i < list.size(); i++){
        if((i - 1) % 4 == 0 && list[i].val() == value){
          return i;
        }
      }
    }
    return -1;
  }

  bool anyIndexIsZero(IntVarArray list, int index){
    if(list[index + 1].val() != 0)
      return ((list[index + 2].val() == 0) and (list[index + 3].val() == 0));
  }

  VI norOperation(Nor* solution, int index){
    IntVarArray list = solution -> getResult();
    if(solution->getDepth() == 0 or anyIndexIsZero(list, index)){
      return getInputs(solution, list[1].val());
    }

    // We find the position of the NOR gate inputs'
    // identifiers (1 -1 2 3  --> Looking where are 2
    // and 3 in the array).
    int index_first = findIndex(list, list[index + 2].val());
    int index_second = findIndex(list, list[index + 3].val());
    VI left, right;
    if(index_first == -1 or index_second == -1)
      return left;
    
    // Get the inputs for the left part. If it
    // is a NOR gate, we iterate again until
    // getting a valid input.
    if(list[index_first + 1].val() != -1){
      left = getInputs(solution, list[index_first + 1].val());
      if(left[0] == -1)
        return left;
    } else{
      left = norOperation(solution, index_first);
    }

    // Same as before with the right side.
    if(list[index_second + 1].val() != -1){
      right = getInputs(solution, list[index_second + 1].val());
      if(right[0] == -1)
        return right;
    } else{
      right = norOperation(solution, index_first);
    }
    return norResult(solution, left, right);
  }

Nor* compare(Nor* first, Nor* second){
  if(first->count_gates() > second->count_gates()){
    return second;
  } else if (first->count_gates() < second->count_gates()){
    return first;
  } else{
    return first;
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
    const int max_depth = 3;
    for (int k = 0; k < pow(2, n); ++k)
    {
      cin >> u;
      g[k] = u;
    }

    for(int j = 0; j < max_depth; j++){
      if(valid_solution){
        valid_solution->print(g);
        break;
      }
      cout << endl << "Looking for a solution with depth " << j << endl;
      Nor* mod = new Nor(g, n, j);
      DFS<Nor> e(mod);
      delete mod;
      while(Nor* s = e.next()){
        bool valid = true;
        s->print(g);
        VI result = norOperation(s, 0);
        if(result != g){
          valid = false;
        }
        if(valid){
          cout << "SOLUTION FOUND" << endl;
          valid_solution = s;
          break;
        }
      }
    }
  }
  catch (Exception e)
  {
    cerr << "Gecode exception: " << e.what() << endl;
    return 1;
  }
  return 0;
}