//
// Created by baj on 2/2/17.
//

#include <climits>
#include "MaxQ0Agent.h"

MaxQ0Agent::MaxQ0Agent(const bool test): HierarchicalAgent(test) {
  reset();

  subtasks_[Root_T] = {Get_T, Put_T, Refuel_T};

  subtasks_[Get_T] = {Pickup_T, NavB_T, NavG_T, NavR_T, NavY_T}; //to be redefined based on input state
  subtasks_[Put_T] = {Putdown_T, NavB_T, NavG_T, NavR_T, NavY_T};  //to be redefined based on input state
  subtasks_[Refuel_T] = {Fillup_T, NavF_T};

  subtasks_[NavB_T] = {North_T, South_T, East_T, West_T};
  subtasks_[NavG_T] = {North_T, South_T, East_T, West_T};
  subtasks_[NavR_T] = {North_T, South_T, East_T, West_T};
  subtasks_[NavY_T] = {North_T, South_T, East_T, West_T};
  subtasks_[NavF_T] = {North_T, South_T, East_T, West_T};
}

void MaxQ0Agent::buildHierarchy(const State &s)
{
  switch (s.passenger()) {
    case 0: subtasks_[Get_T] = {Pickup_T, NavY_T}; break;
    case 1: subtasks_[Get_T] = {Pickup_T, NavR_T}; break;
    case 2: subtasks_[Get_T] = {Pickup_T, NavB_T}; break;
    case 3: subtasks_[Get_T] = {Pickup_T, NavG_T}; break;
    default: assert(0);
  }

  switch (s.destination()) {
    case 0: subtasks_[Put_T] = {Putdown_T, NavY_T}; break;
    case 1: subtasks_[Put_T] = {Putdown_T, NavR_T}; break;
    case 2: subtasks_[Put_T] = {Putdown_T, NavB_T}; break;
    case 3: subtasks_[Put_T] = {Putdown_T, NavG_T}; break;
    default: assert(0);
  }
}

double MaxQ0Agent::run() {
  State state = env()->state();
  buildHierarchy(state);
  MaxQ0(Root_T, state);
  return rewards;
}

MaxQ0Agent::Task MaxQ0Agent::Pi(Task i, const State &s)
{
  std::vector<double> distri(subtasks_[i].size(), -1000000.0);
  for (uint j = 0; j < subtasks_[i].size(); ++j) {
    Task a = Task(subtasks_[i][j]);
    if (IsActiveState(a, s)) {
      distri[j] = Q(i, s, a);
    }
  }

  int best = PolicyFactory::instance().CreatePolicy(test()? PT_Greedy: PT_EpsilonGreedy)->get_action(distri);
  return (Task) subtasks_[i][best];
}

MaxQ0Agent::Task MaxQ0Agent::argmaxQ(Task i, const State &s) {
  std::vector<double> distri(subtasks_[i].size(), -1000000.0);
  for (uint j = 0; j < subtasks_[i].size(); ++j) {
    Task a = Task(subtasks_[i][j]);
    if (IsActiveState(a, s)) {
      distri[j] = Q(i, s, a);
    }
  }

  int best = PolicyFactory::instance().CreatePolicy(PT_Greedy)->get_action(distri);
  return (Task) subtasks_[i][best];
}

double MaxQ0Agent::Q(Task i, const State &s, Task a)
{
  return V(a, s) + ctable_[i][s][a];
}

int MaxQ0Agent::MaxQ0(Task i, State s)
{
  if (IsPrimitive(i)) {
    Action action = TaskToAction(i);
    double reward = env()->step(action);
    vtable_[i][s] = (1.0 - alpha) * vtable_[i][s] + alpha * reward;
    inc(reward);
    return 1;
  }
  else {
    int count = 0;
    while (!IsTerminalState(i, s) && steps < max_steps) {
      Task a = Pi(i, s);
      int N = MaxQ0(a, s);
      State s_prime = env()->state();
      ctable_[i][s][a] = (1 - alpha) * ctable_[i][s][a] + alpha * pow(gamma, N) * V(i, s_prime);
      count += N;
      s = s_prime;
    }
    return count;
  }
}

double MaxQ0Agent::V(Task i, const State &s)
{
  if (IsPrimitive(i)) {
    return vtable_[i][s];
  }
  else {
    return Q(i, s, argmaxQ(i, s));
  }
};

bool MaxQ0Agent::IsPrimitive(Task task)
{
  return task == Pickup_T ||
         task == Putdown_T ||
         task == North_T ||
         task == South_T ||
         task == West_T ||
         task == East_T ||
         task == Fillup_T;
}

bool MaxQ0Agent::IsActiveState(Task task, const State & state)
{
  switch (task) {
    case Root_T: return true;

    case Get_T: return !state.loaded();
    case Put_T: return state.loaded();
    case Refuel_T: return !state.refueled();

    case NavR_T:
    case NavY_T:
    case NavG_T:
    case NavF_T:
    case NavB_T: return true;

    case Fillup_T:
    case Pickup_T:
    case Putdown_T:
    case North_T:
    case South_T:
    case East_T:
    case West_T: return true; //can performed in any state

    default: assert(0); return false;
  }
}

bool MaxQ0Agent::IsTerminalState(Task task, const State & state)
{
  if (state.terminated()) return true;

  switch (task) {
    case Root_T: return state.unloaded();

    case Get_T: return state.loaded();
    case Put_T: return state.unloaded();
    case Refuel_T: return state.refueled();

    case NavR_T: return state.taxiPosition() == TaxiEnv::Model::ins().terminals()[1];
    case NavY_T: return state.taxiPosition() == TaxiEnv::Model::ins().terminals()[0];
    case NavG_T: return state.taxiPosition() == TaxiEnv::Model::ins().terminals()[3];
    case NavB_T: return state.taxiPosition() == TaxiEnv::Model::ins().terminals()[2];
    case NavF_T: return state.taxiPosition() == TaxiEnv::Model::ins().fuelPosition();

    case Pickup_T:
    case Putdown_T:
    case North_T:
    case South_T:
    case East_T:
    case West_T: return true; //terminate in any state

    default: PRINT_VALUE(task); assert(0); return false;
  }
}
