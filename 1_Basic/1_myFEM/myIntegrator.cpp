/*********************************************************************/
/* File:   myIntegrator.cpp                                          */
/* Author: Joachim Schoeberl                                         */
/* Date:   26. Apr. 2009                                             */
/*********************************************************************/

/*
  
My own simple integrators for the Poisson Equation

*/


#include <pybind11/pybind11.h>
#include <fem.hpp>
#include "myElement.hpp"
#include "myIntegrator.hpp"

namespace ngfem
{
  
  /*
    Calculates the element matrix.

    Input is:
    the finite element: fel 
    the geometry of the element: eltrans
    
    Output is:
    the element matrix: elmat
    
    Efficient memorymanagement is provided my locheap
  */
  void MyLaplaceIntegrator ::
  CalcElementMatrix (const FiniteElement & base_fel,
		     const ElementTransformation & eltrans, 
		     FlatMatrix<double> elmat,
		     LocalHeap & lh) const
  {
    static Timer t("MyCalcElementMatrix");
    RegionTracer reg(TaskManager::GetThreadId(), t);
    
    /*
      tell the compiler that we are expecting one of our elements.
      if not, an exception will be raised
    */
    auto & fel = dynamic_cast<const MyBaseElement &> (base_fel);

    // number of element basis functions:
    int ndof = fel.GetNDof();

    elmat = 0;

    Matrix<> dshape_ref(ndof, 2); // gradient on reference element
    Matrix<> dshape(ndof, 2);     // gradient on mapped element

    /*
      get integration rule for element geometry, 
      integration order is 2 times element order
    */
    IntegrationRule ir(fel.ElementType(), 2*fel.Order());

    // loop over integration points
    for (int i = 0 ; i < ir.GetNIP(); i++)
      {
        // calculate Jacobi matrix in the integration point
        MappedIntegrationPoint<2,2> mip(ir[i], eltrans);

        // lambda(x)
        double lam = coef_lambda -> Evaluate (mip);

        /*
          gradient on reference element
          the i-th row of the matrix is the grad of the i-th shape function
        */
        fel.CalcDShape (ir[i], dshape_ref);
        
        // transform it for the mapped element
        dshape = dshape_ref * mip.GetJacobianInverse();
        
        // integration weight and Jacobi determinant
        double fac = mip.IP().Weight() * mip.GetMeasure();

        // elmat_{i,j} += (fac*lam) * InnerProduct (grad shape_i, grad shape_j)
        elmat += (fac*lam) * dshape * Trans(dshape);
      }     
  }

  /*
    Calculates the element vector.

    Input is:
    the finite element: fel 
    the geometry of the element: eltrans
    
    Output is:
    the element vector: elvec
    
    Efficient memorymanagement is provided my locheap
  */
  void MySourceIntegrator ::
  CalcElementVector (const FiniteElement & base_fel,
		     const ElementTransformation & eltrans, 
		     FlatVector<double> elvec,
		     LocalHeap & lh) const
  {
    auto & fel = dynamic_cast<const MyBaseElement&> (base_fel);

    int ndof = fel.GetNDof();

    elvec = 0;

    Vector<> shape(ndof); 

    IntegrationRule ir(fel.ElementType(), 2*fel.Order());

    for (int i = 0 ; i < ir.GetNIP(); i++)
      {
        MappedIntegrationPoint<2,2> mip(ir[i], eltrans);

        double f = coef_f -> Evaluate (mip);

        // calculate shape functions 
        fel.CalcShape (ir[i], shape);
        
        // integration weight and Jacobi determinant
        double fac = mip.IP().Weight() * mip.GetMeasure();

        // elvec_{i} += (fac*f) shape_i
        elvec += (fac*f) * shape;
      }     
  }
}

void ExportMyIntegrator(py::module m)
{
  using namespace ngfem;
  py::class_<MyLaplaceIntegrator, shared_ptr<MyLaplaceIntegrator>, BilinearFormIntegrator>
    (m, "MyLaplace")
    .def(py::init<shared_ptr<CoefficientFunction>>())
    ;
  py::class_<MySourceIntegrator, shared_ptr<MySourceIntegrator>, LinearFormIntegrator>
    (m, "MySource")
    .def(py::init<shared_ptr<CoefficientFunction>>())
    ;
}

