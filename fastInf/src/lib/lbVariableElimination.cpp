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

#include <lbVariableElimination.h> 
using namespace lbLib;

lbVariableElimination::lbVariableElimination(lbModel& model,
						   lbMeasureDispatcher const& disp)
  : _disp(disp)
{
  const lbGraphStruct & graph = model.getGraph();
  int numCliques = graph.getNumOfCliques();
  _measures.resize(numCliques);

  for (cliqIndex index = 0; index < numCliques; index++) {
    _measures[index] = model.getAssignedMeasureForClique(index).duplicate();
  }

  _vars = graph.getVars().getVarsVec();
}

lbVariableElimination::lbVariableElimination(lbRegionModel const& rgnmodel,
					     lbMeasureDispatcher const& disp)
  : _disp(disp) {
  uint  i;
  for (i = 0; i < rgnmodel.getCards().size(); i++) {
    _vars.push_back(i);
  }

  lbAssignedMeasureConstPtrVec ampv = rgnmodel.getAllFactors();
  _measures.resize(ampv.size());

  for (i = 0; i < ampv.size(); i++) {
    _measures[i] = ampv[i]->duplicate();
  }
}

lbAssignedMeasure_ptr lbVariableElimination::prob(varsVec const& remaining)
{
  lbAssignedMeasurePtrVec measures(_measures.size());

  for (uint index = 0; index < _measures.size(); index++) {
    measures[index] = _measures[index]->duplicate();
  }

  for (uint i = 0; i < _vars.size(); i++) {
    bool leaveAlone = false;

    for (uint j = 0; j < remaining.size(); j++) {
      if (remaining[j] == _vars[i]) {
	leaveAlone = true;
	break;
      }
    }

    if (!leaveAlone) {
      marginalizeSingleVariable(measures, _vars[i]);
    }
  }
  
  lbAssignedMeasure_ptr result = multiplyInvolvedMeasures(measures);
  result->normalize();
  return result;
}


bool lbVariableElimination::isVariableInScope(lbAssignedMeasure_ptr mes, rVarIndex var) 
{
    varsVec scope = mes->getVars();

    for (uint j = 0; j < scope.size(); j++) {
      if (scope[j] == var) {
	return true;
      }
    }
    return false;
}

void lbVariableElimination::filterInvolvedMeasures(lbAssignedMeasurePtrVec & measures, 
						      lbAssignedMeasurePtrVec & involved,
						      rVarIndex var) 
{
  lbAssignedMeasurePtrVec::iterator it = measures.begin();

  while (it != measures.end()) {
    lbAssignedMeasure * mes = *it;
    
    if (isVariableInScope(mes, var)) {
      it = measures.erase(it);
      involved.push_back(mes);
    }
    else {
      it++;
    }
  }
}

lbAssignedMeasure_ptr lbVariableElimination::multiplyInvolvedMeasures(lbAssignedMeasurePtrVec & involved)
{
  /* Now we have to multiply them together */
  lbAssignedMeasure *phi = involved.back();
  involved.pop_back();

  while (!involved.empty()) {
    lbAssignedMeasure *nextMeasure = involved.back();
    involved.pop_back();

    lbAssignedMeasure *result = phi->combineMeasures(*nextMeasure, _disp, COMBINE_MULTIPLY);
    delete nextMeasure;
    delete phi;
    phi = result;
  }

  return phi;
}

void lbVariableElimination::marginalizeSingleVariable(lbAssignedMeasurePtrVec & measures, 
							 rVarIndex var)
{
  lbAssignedMeasurePtrVec involved; /* Measures that involve var */
  filterInvolvedMeasures(measures, involved, var);

  if (involved.empty()) {
    cerr << "That's odd.  No measures involved the variable " << var << endl;
    return;
  }

  lbAssignedMeasure *multiplied = multiplyInvolvedMeasures(involved);

  varsVec remaining = multiplied->getVars();
  for (uint i = 0; i < remaining.size(); i++) {
    if (remaining[i] == var) {
      /* Remove the variable we're marginalizing out */
      remaining.erase(remaining.begin() + i);
    }
  }

  measures.push_back(multiplied->marginalize(remaining, _disp));
  delete multiplied;
}
