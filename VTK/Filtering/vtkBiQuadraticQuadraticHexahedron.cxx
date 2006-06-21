/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

//Thanks to Soeren Gebbert  who developed this class and
//integrated it into VTK 5.0.

#include "vtkBiQuadraticQuadraticHexahedron.h"

#include "vtkObjectFactory.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkHexahedron.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkQuadraticEdge.h"
#include "vtkQuadraticQuad.h"
#include "vtkBiQuadraticQuad.h"

vtkCxxRevisionMacro(vtkBiQuadraticQuadraticHexahedron, "$Revision$");
vtkStandardNewMacro(vtkBiQuadraticQuadraticHexahedron);

//----------------------------------------------------------------------------
// Construct the hex with 24 points + 3 extra points for internal
// computation.
vtkBiQuadraticQuadraticHexahedron::vtkBiQuadraticQuadraticHexahedron()
{
  // At times the cell looks like it has 27 points (during interpolation)
  // We initially allocate for 27.
  this->Points->SetNumberOfPoints(27);
  this->PointIds->SetNumberOfIds(27);
  for (int i = 0; i < 27; i++)
    {
    this->Points->SetPoint(i, 0.0, 0.0, 0.0);
    this->PointIds->SetId(i,0);
    }
  this->Points->SetNumberOfPoints(24);
  this->PointIds->SetNumberOfIds(24);

  this->Edge = vtkQuadraticEdge::New();
  this->Face = vtkQuadraticQuad::New();
  this->BiQuadFace = vtkBiQuadraticQuad::New();
  this->Hex = vtkHexahedron::New();

  this->PointData = vtkPointData::New();
  this->CellData = vtkCellData::New();
  this->CellScalars = vtkDoubleArray::New();
  this->CellScalars->SetNumberOfTuples(27);
  this->Scalars = vtkDoubleArray::New();
  this->Scalars->SetNumberOfTuples(8);
}

//----------------------------------------------------------------------------
vtkBiQuadraticQuadraticHexahedron::~vtkBiQuadraticQuadraticHexahedron()
{
  this->Edge->Delete();
  this->Face->Delete();
  this->BiQuadFace->Delete();
  this->Hex->Delete();

  this->PointData->Delete();
  this->CellData->Delete();
  this->Scalars->Delete();
  this->CellScalars->Delete();
}


static int LinearHexs[8][8] = {
    { 0,  8, 24, 11, 16, 22, 26, 20},
    { 8,  1,  9, 24, 22, 17, 21, 26},
    {11, 24, 10,  3, 20, 26, 23, 19},
    {24,  9,  2, 10, 26, 21, 18, 23},
    {16, 22, 26, 20,  4, 12, 25, 15},
    {22, 17, 21, 26, 12,  5, 13, 25},
    {20, 26, 23, 19, 15, 25, 14,  7},
    {26, 21, 18, 23, 25, 13,  6, 14} };

static int HexFaces[6][9] = {
    {0,4,7,3,16,15,19,11, 20},//BiQuadQuad
    {1,2,6,5,9,18,13,17,21},//BiQuadQuad
    {0,1,5,4,8,17,12,16,22}, //BiQuadQuad
    {3,7,6,2,19,14,18,10,23},//BiQuadQuad
    {0,3,2,1,11,10,9,8, 0},//QuadQuad
    {4,5,6,7,12,13,14,15, 0} };//QuadQuad

static int HexEdges[12][3] = { {0,1,8}, {1,2,9}, {3,2,10}, {0,3,11},
                               {4,5,12}, {5,6,13}, {7,6,14}, {4,7,15},
                               {0,4,16}, {1,5,17}, {3,7,19}, {2,6,18} };

static double MidPoints[3][3] = { {0.5,0.5,0.0}, {0.5,0.5,1.0}, {0.5,0.5,0.5} };


//----------------------------------------------------------------------------
vtkCell *vtkBiQuadraticQuadraticHexahedron::GetEdge(int edgeId)
{
  edgeId = (edgeId < 0 ? 0 : (edgeId > 11 ? 11 : edgeId ));

  for (int i=0; i<3; i++)
    {
    this->Edge->PointIds->SetId(i,this->PointIds->GetId(HexEdges[edgeId][i]));
    this->Edge->Points->SetPoint(i,this->Points->GetPoint(HexEdges[edgeId][i]));
    }

  return this->Edge;
}

//----------------------------------------------------------------------------
vtkCell *vtkBiQuadraticQuadraticHexahedron::GetFace(int faceId)
{
  faceId = (faceId < 0 ? 0 : (faceId > 5 ? 5 : faceId ));

  //4 BiQuaduadaticQuads
  if(faceId < 4)
    {
    for (int i=0; i<9; i++)
      {
      this->BiQuadFace->PointIds->SetId(i,this->PointIds->GetId(HexFaces[faceId][i]));
      this->BiQuadFace->Points->SetPoint(i,this->Points->GetPoint(HexFaces[faceId][i]));
      }
    return this->BiQuadFace;
    }
  else
    { //2 QuadraticQuads
    for (int i=0; i<8; i++)
      {
      this->Face->PointIds->SetId(i,this->PointIds->GetId(HexFaces[faceId][i]));
      this->Face->Points->SetPoint(i,this->Points->GetPoint(HexFaces[faceId][i]));
      }
    return this->Face;
    }
}

//----------------------------------------------------------------------------
int *vtkBiQuadraticQuadraticHexahedron::GetFaceArray(int faceId)
{
  return HexFaces[faceId];
}

//----------------------------------------------------------------------------
void vtkBiQuadraticQuadraticHexahedron::Subdivide(vtkPointData *inPd, vtkCellData *inCd, 
                                       vtkIdType cellId, vtkDataArray *cellScalars)
{
  int numMidPts, i, j;
  double weights[24];
  double x[3];
  double s;

  //Copy point and cell attribute data, first make sure it's empty:
  this->PointData->Initialize();
  this->CellData->Initialize();
  this->PointData->CopyAllocate(inPd,27);
  this->CellData->CopyAllocate(inCd,8);
  for (i=0; i<24; i++)
    {
    this->PointData->CopyData(inPd,this->PointIds->GetId(i),i);
    this->CellScalars->SetValue( i, cellScalars->GetTuple1(i));
    }
  this->CellData->CopyData(inCd,cellId,0);

  //Interpolate new values
  double p[3];
  for ( numMidPts=0; numMidPts < 3; numMidPts++ )
    {
    this->InterpolationFunctions(MidPoints[numMidPts], weights);

    x[0] = x[1] = x[2] = 0.0;
    s = 0.0;
    for (i=0; i<24; i++)
      {
      this->Points->GetPoint(i, p);
      for (j=0; j<3; j++)
        {
        x[j] += p[j] * weights[i];
        }
      s += cellScalars->GetTuple1(i) * weights[i];
      }
    this->Points->SetPoint(24+numMidPts,x);
    this->CellScalars->SetValue(24+numMidPts,s);
    this->PointData->InterpolatePoint(inPd, 24+numMidPts, this->PointIds, weights);
    }
}

//----------------------------------------------------------------------------
static const double VTK_DIVERGED = 1.e6;
static const int VTK_HEX_MAX_ITERATION=20;
static const double VTK_HEX_CONVERGED=1.e-03;

int vtkBiQuadraticQuadraticHexahedron::EvaluatePosition(double* x,
                                             double* closestPoint,
                                             int& subId, double pcoords[3],
                                             double& dist2, double *weights)
{
  int iteration, converged;
  double  params[3];
  double  fcol[3], rcol[3], scol[3], tcol[3];
  int i, j;
  double  d, pt[3];
  double derivs[72];

  //  set initial position for Newton's method
  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = params[0] = params[1] = params[2]=0.5;

  //  enter iteration loop
  for (iteration=converged=0;
       !converged && (iteration < VTK_HEX_MAX_ITERATION);  iteration++)
    {
    //  calculate element interpolation functions and derivatives
    this->InterpolationFunctions(pcoords, weights);
    this->InterpolationDerivs(pcoords, derivs);

    //  calculate newton functions
    for (i=0; i<3; i++)
      {
      fcol[i] = rcol[i] = scol[i] = tcol[i] = 0.0;
      }
    for (i=0; i<24; i++)
      {
      this->Points->GetPoint(i, pt);
      for (j=0; j<3; j++)
        {
        fcol[j] += pt[j] * weights[i];
        rcol[j] += pt[j] * derivs[i];
        scol[j] += pt[j] * derivs[i+24];
        tcol[j] += pt[j] * derivs[i+48];
        }
      }

    for (i=0; i<3; i++)
      {
      fcol[i] -= x[i];
      }

    //  compute determinants and generate improvements
    d=vtkMath::Determinant3x3(rcol,scol,tcol);
    if ( fabs(d) < 1.e-20)
      {
      return -1;
      }

    pcoords[0] = params[0] - 0.5*vtkMath::Determinant3x3 (fcol,scol,tcol) / d;
    pcoords[1] = params[1] - 0.5*vtkMath::Determinant3x3 (rcol,fcol,tcol) / d;
    pcoords[2] = params[2] - 0.5*vtkMath::Determinant3x3 (rcol,scol,fcol) / d;

    //  check for convergence
    if ( ((fabs(pcoords[0]-params[0])) < VTK_HEX_CONVERGED) &&
         ((fabs(pcoords[1]-params[1])) < VTK_HEX_CONVERGED) &&
         ((fabs(pcoords[2]-params[2])) < VTK_HEX_CONVERGED) )
      {
      converged = 1;
      }

    // Test for bad divergence (S.Hirschberg 11.12.2001)
    else if ((fabs(pcoords[0]) > VTK_DIVERGED) ||
             (fabs(pcoords[1]) > VTK_DIVERGED) ||
             (fabs(pcoords[2]) > VTK_DIVERGED))
      {
      return -1;
      }

    //  if not converged, repeat
    else
      {
      params[0] = pcoords[0];
      params[1] = pcoords[1];
      params[2] = pcoords[2];
      }
    }

  //  if not converged, set the parametric coordinates to arbitrary values
  //  outside of element
  if ( !converged )
    {
    return -1;
    }

  this->InterpolationFunctions(pcoords, weights);

  if ( pcoords[0] >= -0.001 && pcoords[0] <= 1.001 &&
  pcoords[1] >= -0.001 && pcoords[1] <= 1.001 &&
  pcoords[2] >= -0.001 && pcoords[2] <= 1.001 )
    {
    if (closestPoint)
      {
      closestPoint[0] = x[0]; closestPoint[1] = x[1]; closestPoint[2] = x[2];
      dist2 = 0.0; //inside hexahedron
      }
    return 1;
    }
  else
    {
    double pc[3], w[24];
    if (closestPoint)
      {
      for (i=0; i<3; i++) //only approximate, not really true for warped hexa
        {
        if (pcoords[i] < 0.0)
          {
          pc[i] = 0.0;
          }
        else if (pcoords[i] > 1.0)
          {
          pc[i] = 1.0;
          }
        else
          {
          pc[i] = pcoords[i];
          }
        }
      this->EvaluateLocation(subId, pc, closestPoint, (double *)w);
      dist2 = vtkMath::Distance2BetweenPoints(closestPoint,x);
      }
    return 0;
    }
}

//----------------------------------------------------------------------------
void vtkBiQuadraticQuadraticHexahedron::EvaluateLocation(int& vtkNotUsed(subId),
                                              double pcoords[3],
                                              double x[3], double *weights)
{
  int i, j;
  double pt[3];

  this->InterpolationFunctions(pcoords, weights);

  x[0] = x[1] = x[2] = 0.0;
  for (i=0; i<24; i++)
    {
    this->Points->GetPoint(i, pt);
    for (j=0; j<3; j++)
      {
      x[j] += pt[j] * weights[i];
      }
    }
}

//----------------------------------------------------------------------------
int vtkBiQuadraticQuadraticHexahedron::CellBoundary(int subId, double pcoords[3],
                                         vtkIdList *pts)
{
  return this->Hex->CellBoundary(subId, pcoords, pts);
}

//----------------------------------------------------------------------------
void vtkBiQuadraticQuadraticHexahedron::Contour(double value,
                                     vtkDataArray* cellScalars,
                                     vtkPointLocator* locator,
                                     vtkCellArray *verts,
                                     vtkCellArray* lines,
                                     vtkCellArray* polys,
                                     vtkPointData* inPd,
                                     vtkPointData* outPd,
                                     vtkCellData* inCd,
                                     vtkIdType cellId,
                                     vtkCellData* outCd)
{
  //subdivide into 8 linear hexs
  this->Subdivide(inPd,inCd,cellId, cellScalars);

  //contour each linear quad separately
  for (int i=0; i<8; i++) // For each subdivided hexahedron
    {
    for (int j=0; j<8; j++) // For each of the eight vertices of the hexhedron
      {
      this->Hex->Points->SetPoint(j,this->Points->GetPoint(LinearHexs[i][j]));
      this->Hex->PointIds->SetId(j,LinearHexs[i][j]);
      this->Scalars->SetValue(j,this->CellScalars->GetValue(LinearHexs[i][j]));
      }
    this->Hex->Contour(value,this->Scalars,locator,verts,lines,polys,
                       this->PointData,outPd,this->CellData,cellId,outCd);
    }
}

//----------------------------------------------------------------------------
// Line-hex intersection. Intersection has to occur within [0,1] parametric
// coordinates and with specified tolerance.
int vtkBiQuadraticQuadraticHexahedron::IntersectWithLine(double* p1, double* p2,
                                              double tol, double& t,
                                              double* x, double* pcoords,
                                              int& subId)
{
  int intersection = 0;
  double tTemp;
  double pc[3], xTemp[3];
  int faceNum, status = 0;

  t = VTK_DOUBLE_MAX;
  for (faceNum=0; faceNum<6; faceNum++)
    {
    status = 0;
    //4 BiQuaduadaticQuads
    if(faceNum < 4)
      {
      for (int i=0; i<9; i++)
        {
        this->BiQuadFace->PointIds->SetId(i,
          this->PointIds->GetId(HexFaces[faceNum][i]));
        this->BiQuadFace->Points->SetPoint(i,
          this->Points->GetPoint(HexFaces[faceNum][i]));
        }
      status = this->BiQuadFace->IntersectWithLine(p1, p2, tol, tTemp, xTemp, pc, subId);

      }
    else
      { //2 QuadraticQuads
      for (int i=0; i<8; i++)
        {
        this->Face->PointIds->SetId(i,this->PointIds->GetId(HexFaces[faceNum][i]));
        this->Face->Points->SetPoint(i,this->Points->GetPoint(HexFaces[faceNum][i]));
        }
      status = this->Face->IntersectWithLine(p1, p2, tol, tTemp, xTemp, pc, subId);
      }

    if (status)
      {
      intersection = 1;
      if ( tTemp < t )
        {
        t = tTemp;
        x[0] = xTemp[0]; x[1] = xTemp[1]; x[2] = xTemp[2];
        switch (faceNum)
          {
        case 0:
          pcoords[0] = 0.0; pcoords[1] = pc[1]; pcoords[2] = pc[0];
          break;

        case 1:
          pcoords[0] = 1.0; pcoords[1] = pc[0]; pcoords[2] = pc[1];
          break;

        case 2:
          pcoords[0] = pc[0]; pcoords[1] = 0.0; pcoords[2] = pc[1];
          break;

        case 3:
          pcoords[0] = pc[1]; pcoords[1] = 1.0; pcoords[2] = pc[0];
          break;

        case 4:
          pcoords[0] = pc[1]; pcoords[1] = pc[0]; pcoords[2] = 0.0;
          break;

        case 5:
          pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 1.0;
          break;
          }
        }
      }
    }
  return intersection;
}

//----------------------------------------------------------------------------
int vtkBiQuadraticQuadraticHexahedron::Triangulate(int vtkNotUsed(index),
                                        vtkIdList *ptIds, vtkPoints *pts)
{
  pts->Reset();
  ptIds->Reset();

  ptIds->InsertId(0,this->PointIds->GetId(0));
  pts->InsertPoint(0,this->Points->GetPoint(0));

  ptIds->InsertId(1,this->PointIds->GetId(1));
  pts->InsertPoint(1,this->Points->GetPoint(1));

  return 1;
}

//----------------------------------------------------------------------------
// Given parametric coordinates compute inverse Jacobian transformation
// matrix. Returns 9 elements of 3x3 inverse Jacobian plus interpolation
// function derivatives.
void vtkBiQuadraticQuadraticHexahedron::JacobianInverse(double pcoords[3],
                                             double **inverse,
                                             double derivs[72])
{
  int i, j;
  double *m[3], m0[3], m1[3], m2[3];
  double x[3];

  // compute interpolation function derivatives
  this->InterpolationDerivs(pcoords, derivs);

  // create Jacobian matrix
  m[0] = m0; m[1] = m1; m[2] = m2;
  for (i=0; i < 3; i++) //initialize matrix
    {
    m0[i] = m1[i] = m2[i] = 0.0;
    }

  for ( j=0; j < 24; j++ )
    {
    this->Points->GetPoint(j, x);
    for ( i=0; i < 3; i++ )
      {
      m0[i] += x[i] * derivs[j];
      m1[i] += x[i] * derivs[24 + j];
      m2[i] += x[i] * derivs[48 + j];
      }
    }

  // now find the inverse
  if ( vtkMath::InvertMatrix(m,inverse,3) == 0 )
    {
    vtkErrorMacro(<<"Jacobian inverse not found");
    return;
    }
}

//----------------------------------------------------------------------------
void vtkBiQuadraticQuadraticHexahedron::Derivatives(int vtkNotUsed(subId),
                                         double pcoords[3], double *values,
                                         int dim, double *derivs)
{
  double *jI[3], j0[3], j1[3], j2[3];
  double functionDerivs[72], sum[3];
  int i, j, k;

  // compute inverse Jacobian and interpolation function derivatives
  jI[0] = j0; jI[1] = j1; jI[2] = j2;
  this->JacobianInverse(pcoords, jI, functionDerivs);

  // now compute derivates of values provided
  for (k=0; k < dim; k++) //loop over values per vertex
    {
    sum[0] = sum[1] = sum[2] = 0.0;
    for ( i=0; i < 24; i++) //loop over interp. function derivatives
      {
      sum[0] += functionDerivs[i] * values[dim*i + k];
      sum[1] += functionDerivs[24 + i] * values[dim*i + k];
      sum[2] += functionDerivs[48 + i] * values[dim*i + k];
      }
    for (j=0; j < 3; j++) //loop over derivative directions
      {
      derivs[3*k + j] = sum[0]*jI[j][0] + sum[1]*jI[j][1] + sum[2]*jI[j][2];
      }
    }
}


//----------------------------------------------------------------------------
// Clip this quadratic hex using scalar value provided. Like contouring,
// except that it cuts the hex to produce tetrahedra.
void vtkBiQuadraticQuadraticHexahedron::Clip(double value,
                                  vtkDataArray* cellScalars,
                                  vtkPointLocator* locator, vtkCellArray* tets,
                                  vtkPointData* inPd, vtkPointData* outPd,
                                  vtkCellData* inCd, vtkIdType cellId,
                                  vtkCellData* outCd, int insideOut)
{
  //create eight linear hexes
  this->Subdivide(inPd,inCd,cellId,cellScalars);

  //contour each linear hex separately
  for (int i=0; i<8; i++) // For each subdivided hexahedron
    {
    for (int j=0; j<8; j++) // For each of the eight vertices of the hexhedron
      {
      this->Hex->Points->SetPoint(j,this->Points->GetPoint(LinearHexs[i][j]));
      this->Hex->PointIds->SetId(j,LinearHexs[i][j]);
      this->Scalars->SetValue(j,this->CellScalars->GetValue(LinearHexs[i][j]));
      }
    this->Hex->Clip(value,this->Scalars,locator,tets,this->PointData,outPd,
                    this->CellData,cellId,outCd,insideOut);
    }
}

//----------------------------------------------------------------------------
// Compute interpolation functions for the twenty four nodes.
void vtkBiQuadraticQuadraticHexahedron::InterpolationFunctions(double pcoords[3],
                                                    double weights[24])
{
  //VTK needs parametric coordinates to be between (0,1). Isoparametric
  //shape functions are formulated between (-1,1). Here we do a
  //coordinate system conversion from (0,1) to (-1,1).
  double x = 2.0*(pcoords[0]-0.5);
  double y = 2.0*(pcoords[1]-0.5);
  double z = 2.0*(pcoords[2]-0.5);

  //The eight corner points
  weights[0] = ( 0.25*(x*(1-x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  weights[1] = (-0.25*(x*(1+x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  weights[2] = ( 0.25*(x*(1+x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  weights[3] = (-0.25*(x*(1-x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  weights[4] = ( 0.25*(x*(1-x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  weights[5] = (-0.25*(x*(1+x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  weights[6] = ( 0.25*(x*(1+x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  weights[7] = (-0.25*(x*(1-x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*z*(1+z));

  //The mid-edge nodes
  weights[8] =  0.5*((1+x)*(1-x))*(1-y) *(-0.5*z*(1-z));
  weights[9] =  0.5*((1+y)*(1-y))*(1+x) *(-0.5*z*(1-z));
  weights[10] = 0.5*((1+x)*(1-x))*(1+y) *(-0.5*z*(1-z));
  weights[11] = 0.5*((1+y)*(1-y))*(1-x) *(-0.5*z*(1-z));
  weights[12] = 0.5*((1+x)*(1-x))*(1-y) *( 0.5*z*(1+z));
  weights[13] = 0.5*((1+y)*(1-y))*(1+x) *( 0.5*z*(1+z));
  weights[14] = 0.5*((1+x)*(1-x))*(1+y) *( 0.5*z*(1+z));
  weights[15] = 0.5*((1+y)*(1-y))*(1-x) *( 0.5*z*(1+z));
  weights[16] =( 0.25*(x*(1-x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((1+z)*(1-z));
  weights[17] =(-0.25*(x*(1+x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((1+z)*(1-z));
  weights[18] =( 0.25*(x*(1+x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((1+z)*(1-z));
  weights[19] =(-0.25*(x*(1-x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((1+z)*(1-z));

  //Face center Nodes in xz and yz direction
  weights[22] = 0.5*((1+x)*(1-x))*(1-y)  *((1+z)*(1-z));
  weights[21] = 0.5*((1+y)*(1-y))*(1+x)  *((1+z)*(1-z));
  weights[23] = 0.5*((1+x)*(1-x))*(1+y)  *((1+z)*(1-z));
  weights[20] = 0.5*((1+y)*(1-y))*(1-x)  *((1+z)*(1-z));
}

//----------------------------------------------------------------------------
// Derivatives in parametric space.
void vtkBiQuadraticQuadraticHexahedron::InterpolationDerivs(double pcoords[3],
                                                 double derivs[72])
{
  //VTK needs parametric coordinates to be between (0,1). Isoparametric
  //shape functions are formulated between (-1,1). Here we do a
  //coordinate system conversion from (0,1) to (-1,1).
  double x = 2.0*(pcoords[0]-0.5);
  double y = 2.0*(pcoords[1]-0.5);
  double z = 2.0*(pcoords[2]-0.5);

  //x-direction 
  //The eight corner points
  derivs[0]  =( 0.25*(1-2*x)*(y*(1-y)) - 0.25*(-2*x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  derivs[1]  =(-0.25*(1+2*x)*(y*(1-y)) - 0.25*(-2*x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  derivs[2]  =( 0.25*(1+2*x)*(y*(1+y)) - 0.25*(-2*x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  derivs[3]  =(-0.25*(1-2*x)*(y*(1+y)) - 0.25*(-2*x)*(1+y)*(1-y))*(-0.5*z*(1-z));
  derivs[4]  =( 0.25*(1-2*x)*(y*(1-y)) - 0.25*(-2*x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  derivs[5]  =(-0.25*(1+2*x)*(y*(1-y)) - 0.25*(-2*x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  derivs[6]  =( 0.25*(1+2*x)*(y*(1+y)) - 0.25*(-2*x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  derivs[7]  =(-0.25*(1-2*x)*(y*(1+y)) - 0.25*(-2*x)*(1+y)*(1-y))*( 0.5*z*(1+z));
  //The mid-edge nodes
  derivs[8]  = 0.5*((-2*x))*(1-y) *      (-0.5*z*(1-z));
  derivs[9]  = 0.5*((1+y)*(1-y))       * (-0.5*z*(1-z));
  derivs[10] = 0.5*((-2*x))*(1+y) *      (-0.5*z*(1-z));
  derivs[11] =-0.5*((1+y)*(1-y))       * (-0.5*z*(1-z));
  derivs[12] = 0.5*((-2*x))*(1-y) *      ( 0.5*z*(1+z));
  derivs[13] = 0.5*((1+y)*(1-y))       * ( 0.5*z*(1+z));
  derivs[14] = 0.5*((-2*x))*(1+y) *      ( 0.5*z*(1+z));
  derivs[15] =-0.5*((1+y)*(1-y))       * ( 0.5*z*(1+z));
  derivs[16] =( 0.25*(1-2*x)*(y*(1-y)) - 0.25*(-2*x)*(1+y)*(1-y)) *((1+z)*(1-z));
  derivs[17] =(-0.25*(1+2*x)*(y*(1-y)) - 0.25*(-2*x)*(1+y)*(1-y)) *((1+z)*(1-z));
  derivs[18] =( 0.25*(1+2*x)*(y*(1+y)) - 0.25*(-2*x)*(1+y)*(1-y)) *((1+z)*(1-z));
  derivs[19] =(-0.25*(1-2*x)*(y*(1+y)) - 0.25*(-2*x)*(1+y)*(1-y)) *((1+z)*(1-z));
  //Face center Nodes in xz and yz direction
  derivs[20] = 0.5*((-2*x))*(1-y)  *((1+z)*(1-z));
  derivs[21] = 0.5*((1+y)*(1-y))   *((1+z)*(1-z));
  derivs[22] = 0.5*((-2*x))*(1+y)  *((1+z)*(1-z));
  derivs[23] =-0.5*((1+y)*(1-y))   *((1+z)*(1-z));
  
  //y-direction
  //The eight Corner points
  derivs[24] =( 0.25*(x*(1-x))*(1-2*y) - 0.25*(1+x)*(1-x)*(-2*y))*(-0.5*z*(1-z));
  derivs[25] =(-0.25*(x*(1+x))*(1-2*y) - 0.25*(1+x)*(1-x)*(-2*y))*(-0.5*z*(1-z));
  derivs[26] =( 0.25*(x*(1+x))*(1+2*y) - 0.25*(1+x)*(1-x)*(-2*y))*(-0.5*z*(1-z));
  derivs[27] =(-0.25*(x*(1-x))*(1+2*y) - 0.25*(1+x)*(1-x)*(-2*y))*(-0.5*z*(1-z));
  derivs[28] =( 0.25*(x*(1-x))*(1-2*y) - 0.25*(1+x)*(1-x)*(-2*y))*( 0.5*z*(1+z));
  derivs[29] =(-0.25*(x*(1+x))*(1-2*y) - 0.25*(1+x)*(1-x)*(-2*y))*( 0.5*z*(1+z));
  derivs[30] =( 0.25*(x*(1+x))*(1+2*y) - 0.25*(1+x)*(1-x)*(-2*y))*( 0.5*z*(1+z));
  derivs[31] =(-0.25*(x*(1-x))*(1+2*y) - 0.25*(1+x)*(1-x)*(-2*y))*( 0.5*z*(1+z));
  //The mid-edge nodes
  derivs[32] =-0.5*((1+x)*(1-x))  *(-0.5*z*(1-z));
  derivs[33] = 0.5*((-2*y))*(1+x) *(-0.5*z*(1-z));
  derivs[34] = 0.5*((1+x)*(1-x))  *(-0.5*z*(1-z));
  derivs[35] = 0.5*((-2*y))*(1-x) *(-0.5*z*(1-z));
  derivs[36] =-0.5*((1+x)*(1-x))  *( 0.5*z*(1+z));
  derivs[37] = 0.5*((-2*y))*(1+x) *( 0.5*z*(1+z));
  derivs[38] = 0.5*((1+x)*(1-x))  *( 0.5*z*(1+z));
  derivs[39] = 0.5*((-2*y))*(1-x) *( 0.5*z*(1+z));
  derivs[40] =( 0.25*(x*(1-x))*(1-2*y) - 0.25*(1+x)*(1-x)*(-2*y)) *((1+z)*(1-z));
  derivs[41] =(-0.25*(x*(1+x))*(1-2*y) - 0.25*(1+x)*(1-x)*(-2*y)) *((1+z)*(1-z));
  derivs[42] =( 0.25*(x*(1+x))*(1+2*y) - 0.25*(1+x)*(1-x)*(-2*y)) *((1+z)*(1-z));
  derivs[43] =(-0.25*(x*(1-x))*(1+2*y) - 0.25*(1+x)*(1-x)*(-2*y)) *((1+z)*(1-z));
  //Face center Nodes in xz and yz direction
  derivs[44] =-0.5*((1+x)*(1-x))        * ((1+z)*(1-z));
  derivs[45] = 0.5*((-2*y))*(1+x)  *      ((1+z)*(1-z));
  derivs[46] = 0.5*((1+x)*(1-x))        * ((1+z)*(1-z));
  derivs[47] = 0.5*((-2*y))*(1-x)  *      ((1+z)*(1-z));
  
  //z-direction
  //The eight corner points
  derivs[48] =( 0.25*(x*(1-x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*(1-2*z));
  derivs[49] =(-0.25*(x*(1+x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*(1-2*z));
  derivs[50] =( 0.25*(x*(1+x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*(1-2*z));
  derivs[51] =(-0.25*(x*(1-x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*(-0.5*(1-2*z));
  derivs[52] =( 0.25*(x*(1-x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*(1+2*z));
  derivs[53] =(-0.25*(x*(1+x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*(1+2*z));
  derivs[54] =( 0.25*(x*(1+x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*(1+2*z));
  derivs[55] =(-0.25*(x*(1-x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y))*( 0.5*(1+2*z));
  //The mid-edge nodes
  derivs[56] = 0.5*((1+x)*(1-x))*(1-y) *(-0.5*(1-2*z));
  derivs[57] = 0.5*((1+y)*(1-y))*(1+x) *(-0.5*(1-2*z));
  derivs[58] = 0.5*((1+x)*(1-x))*(1+y) *(-0.5*(1-2*z));
  derivs[59] = 0.5*((1+y)*(1-y))*(1-x) *(-0.5*(1-2*z));
  derivs[60] = 0.5*((1+x)*(1-x))*(1-y) *( 0.5*(1+2*z));
  derivs[61] = 0.5*((1+y)*(1-y))*(1+x) *( 0.5*(1+2*z));
  derivs[62] = 0.5*((1+x)*(1-x))*(1+y) *( 0.5*(1+2*z));
  derivs[63] = 0.5*((1+y)*(1-y))*(1-x) *( 0.5*(1+2*z));
  derivs[64] =( 0.25*(x*(1-x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((-2*z));
  derivs[65] =(-0.25*(x*(1+x))*(y*(1-y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((-2*z));
  derivs[66] =( 0.25*(x*(1+x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((-2*z));
  derivs[67] =(-0.25*(x*(1-x))*(y*(1+y)) - 0.25*(1+x)*(1-x)*(1+y)*(1-y)) *((-2*z));
  //Face center Nodes in xz and yz direction
  derivs[68] =0.5*((1+x)*(1-x))*(1-y)  *((-2*z));
  derivs[69] =0.5*((1+y)*(1-y))*(1+x)  *((-2*z));
  derivs[70] =0.5*((1+x)*(1-x))*(1+y)  *((-2*z));
  derivs[71] =0.5*((1+y)*(1-y))*(1-x)  *((-2*z));
}

//----------------------------------------------------------------------------
static double vtkQHexCellPCoords[72] ={0.0,0.0,0.0, 1.0,0.0,0.0, 1.0,1.0,0.0,
                                       0.0,1.0,0.0, 0.0,0.0,1.0, 1.0,0.0,1.0,
                                       1.0,1.0,1.0, 0.0,1.0,1.0, 0.5,0.0,0.0,
                                       1.0,0.5,0.0, 0.5,1.0,0.0, 0.0,0.5,0.0,
                                       0.5,0.0,1.0, 1.0,0.5,1.0, 0.5,1.0,1.0,
                                       0.0,0.5,1.0, 0.0,0.0,0.5, 1.0,0.0,0.5,
                                       1.0,1.0,0.5, 0.0,1.0,0.5,
                                       0.0,0.5,0.5,  // 20
                                       1.0,0.5,0.5,  // 21
                                       0.5,0.0,0.5,  // 22
                                       0.5,1.0,0.5}; // 23

double *vtkBiQuadraticQuadraticHexahedron::GetParametricCoords()
{
  return vtkQHexCellPCoords;
}

//----------------------------------------------------------------------------
void vtkBiQuadraticQuadraticHexahedron::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Edge:\n";
  this->Edge->PrintSelf(os,indent.GetNextIndent());
  os << indent << "Face:\n";
  this->Face->PrintSelf(os,indent.GetNextIndent());
  this->BiQuadFace->PrintSelf(os,indent.GetNextIndent());
  os << indent << "Hex:\n";
  this->Hex->PrintSelf(os,indent.GetNextIndent());
  os << indent << "PointData:\n";
  this->PointData->PrintSelf(os,indent.GetNextIndent());
  os << indent << "CellData:\n";
  this->CellData->PrintSelf(os,indent.GetNextIndent());
  os << indent << "Scalars:\n";
  this->Scalars->PrintSelf(os,indent.GetNextIndent());
}
