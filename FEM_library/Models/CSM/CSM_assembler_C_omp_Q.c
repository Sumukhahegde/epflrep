/*   This file is part of redbKIT.
 *   Copyright (c) 2016, Ecole Polytechnique Federale de Lausanne (EPFL)
 *   Author: Federico Negri <federico.negri@epfl.ch>
 */

#include "mex.h"
#include <stdio.h>
#include <math.h>
#include "blas.h"
#include <string.h>
#define INVJAC(i,j,k) invjac[i+(j+k*dim)*noe]
#define GRADREFPHI(i,j,k) gradrefphi[i+(j+k*NumQuadPoints)*nln]
#ifdef _OPENMP
#include <omp.h>
#else
#warning "OpenMP not enabled. Compile with mex CSM_assembler_C_omp.c CFLAGS="\$CFLAGS -fopenmp" LDFLAGS="\$LDFLAGS -fopenmp""
#endif

/*************************************************************************/

double Mdot(int dim, double X[dim][dim], double Y[dim][dim])
{
    int d1, d2;
    double Z = 0;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            Z = Z + X[d1][d2] * Y[d1][d2];
        }
    }
    return Z;
}

/*************************************************************************/
void MatrixSum(int dim, double X[dim][dim], double Y[dim][dim] )
{
    int d1, d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            X[d1][d2] = X[d1][d2] + Y[d1][d2];
        }
    }
}
/*************************************************************************/
void MatrixSumAlpha(int dim, double alpha, double X[dim][dim], double beta, double Y[dim][dim], double result[dim][dim] )
{
    int d1, d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = alpha * X[d1][d2] + beta * Y[d1][d2];
        }
    }
}
/*************************************************************************/
double MatrixDeterminant2(int dim, double A[dim][dim])
{
    return A[0][0]*A[1][1]-A[0][1]*A[1][0];
}
/*************************************************************************/
double MatrixDeterminant3(int dim, double A[dim][dim])
{
    return A[0][0]*(A[1][1]*A[2][2]-A[2][1]*A[1][2]) -A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])  +A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
}
/*************************************************************************/
double MatrixDeterminant(int dim, double A[dim][dim])
{
    double determinant = 0;
    if ( dim == 2 )
    {
        determinant = MatrixDeterminant2(2, A);
    }
    
    if ( dim == 3 )
    {
        determinant = MatrixDeterminant3(3, A);
    }
        
    return determinant;
}
/*************************************************************************/
void MatrixInvT(int dim, double A[dim][dim], double invAT[dim][dim] )
{
    if ( dim == 2 )
    {
        double det = MatrixDeterminant2(2, A);
        
        double invdet = 1/det;
        
        invAT[0][0] =   A[1][1]*invdet;
        invAT[1][0] =  -A[0][1]*invdet;
        invAT[0][1] =  -A[1][0]*invdet;
        invAT[1][1] =   A[0][0]*invdet;
    }
    
    if ( dim == 3 )
    {
        double det = MatrixDeterminant3(3, A);
        
        double invdet = 1/det;
        
        invAT[0][0] =  (A[1][1]*A[2][2]-A[2][1]*A[1][2])*invdet;
        invAT[1][0] = -(A[0][1]*A[2][2]-A[0][2]*A[2][1])*invdet;
        invAT[2][0] =  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
        invAT[0][1] = -(A[1][0]*A[2][2]-A[1][2]*A[2][0])*invdet;
        invAT[1][1] =  (A[0][0]*A[2][2]-A[0][2]*A[2][0])*invdet;
        invAT[2][1] = -(A[0][0]*A[1][2]-A[1][0]*A[0][2])*invdet;
        invAT[0][2] =  (A[1][0]*A[2][1]-A[2][0]*A[1][1])*invdet;
        invAT[1][2] = -(A[0][0]*A[2][1]-A[2][0]*A[0][1])*invdet;
        invAT[2][2] =  (A[0][0]*A[1][1]-A[1][0]*A[0][1])*invdet;
    }
    
}
/*************************************************************************/
void MatrixProduct(int dim, double X[dim][dim], double Y[dim][dim], double result[dim][dim] )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                result[d1][d2] = result[d1][d2] + X[d1][d3]*Y[d3][d2];
            }
        }
    }
}
/*************************************************************************/
void MatrixScalar(int dim, double alpha, double X[dim][dim], double result[dim][dim] )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = alpha * X[d1][d2];
        }
    }
}
/*************************************************************************/
void MatrixProductAlpha(int dim, double alpha, double X[dim][dim], double Y[dim][dim], double result[dim][dim] )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                result[d1][d2] = result[d1][d2] + X[d1][d3]*Y[d3][d2];
            }
            result[d1][d2] = alpha * result[d1][d2];
        }
    }
}
/*************************************************************************/
void MatrixProductAlphaT1(int dim, double alpha, double X[dim][dim], double Y[dim][dim], double result[dim][dim] )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                result[d1][d2] = result[d1][d2] + X[d3][d1]*Y[d3][d2];
            }
            result[d1][d2] = alpha * result[d1][d2];
        }
    }
}
/*************************************************************************/
void MatrixProductAlphaT2(int dim, double alpha, double X[dim][dim], double Y[dim][dim], double result[dim][dim] )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                result[d1][d2] = result[d1][d2] + X[d1][d3]*Y[d2][d3];
            }
            result[d1][d2] = alpha * result[d1][d2];
        }
    }
}
/*************************************************************************/
void MatrixProductAlphaT3(int dim, double alpha, double X[dim][dim], double Y[dim][dim], double result[dim][dim] )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                result[d1][d2] = result[d1][d2] + X[d3][d1]*Y[d2][d3];
            }
            result[d1][d2] = alpha * result[d1][d2];
        }
    }
}
/*************************************************************************/
void MatrixProductQ1(int dim, int numQuadPoints, double X[dim][dim][numQuadPoints], double Y[dim][dim], double result[dim][dim], int q )
{
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            result[d1][d2] = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                result[d1][d2] = result[d1][d2] + X[d1][d3][q]*Y[d3][d2];
            }
        }
    }
}
/*************************************************************************/
double Trace(int dim, double X[dim][dim])
{
    double T = 0;
    int d1;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        T = T + X[d1][d1];
    }
    return T;
}
/*************************************************************************/
double TraceQ(int dim, int numQuadPoints, double X[dim][dim][numQuadPoints], int q)
{
    double T = 0;
    int d1;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        T = T + X[d1][d1][q];
    }
    return T;
}
/*************************************************************************/
void compute_GreenStrainTensor(int dim, int numQuadPoints, double F[dim][dim][numQuadPoints], double Id[dim][dim], double E[dim][dim][numQuadPoints], int q )
{
    
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            double tmp = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                tmp = tmp + F[d3][d1][q] * F[d3][d2][q];
            }
            E[d1][d2][q] = 0.5 * ( tmp - Id[d1][d2] );
        }
    }
}
/*************************************************************************/
void compute_DerGreenStrainTensor(int dim, int numQuadPoints, double F[dim][dim][numQuadPoints], double dF[dim][dim], double dE[dim][dim], int q )
{
    
    int d1, d2, d3;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            double tmp1 = 0;
            double tmp2 = 0;
            for (d3 = 0; d3 < dim; d3 = d3 + 1 )
            {
                tmp1 = tmp1 + dF[d3][d1] * F[d3][d2][q];
                tmp2 = tmp2 + F[d3][d1][q]  * dF[d3][d2];
            }
            dE[d1][d2] = 0.5 * ( tmp1 + tmp2 );
        }
    }
}
/*************************************************************************/

void LinearElasticMaterial(mxArray* plhs[], const mxArray* prhs[])
{
    
    double* dim_ptr = mxGetPr(prhs[0]);
    int dim     = (int)(dim_ptr[0]);
    int noe     = mxGetN(prhs[4]);
    double* nln_ptr = mxGetPr(prhs[5]);
    int nln     = (int)(nln_ptr[0]);
    int numRowsElements  = mxGetM(prhs[4]);
    int nln2    = nln*nln;
    
    plhs[0] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[2] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[3] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    plhs[4] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    
    double* myArows    = mxGetPr(plhs[0]);
    double* myAcols    = mxGetPr(plhs[1]);
    double* myAcoef    = mxGetPr(plhs[2]);
    double* myRrows    = mxGetPr(plhs[3]);
    double* myRcoef    = mxGetPr(plhs[4]);
    
    int k,l;
    int q;
    int NumQuadPoints     = mxGetN(prhs[6]);
    int NumNodes          = (int)(mxGetM(prhs[3]) / dim);
    
    double* U_h   = mxGetPr(prhs[3]);
    double* w   = mxGetPr(prhs[6]);
    double* invjac = mxGetPr(prhs[7]);
    double* detjac = mxGetPr(prhs[8]);
    double* phi = mxGetPr(prhs[9]);
    double* gradrefphi = mxGetPr(prhs[10]);
    
    double gradphi[dim][nln][NumQuadPoints];
    double* elements  = mxGetPr(prhs[4]);
    
    double GradV[dim][dim];
    double GradU[dim][dim];
    double GradUh[dim][dim][NumQuadPoints];
    
    double Id[dim][dim];
    int d1,d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            Id[d1][d2] = 0;
            if (d1==d2)
            {
                Id[d1][d2] = 1;
            }
        }
    }
    
    double F[dim][dim];
    double EPS[dim][dim];
    double dP[dim][dim];
    double P_Uh[dim][dim];
    
    double* material_param = mxGetPr(prhs[2]);
    double Young = material_param[0];
    double Poisson = material_param[1];
    double mu = Young / (2 + 2 * Poisson);
    double lambda =  Young * Poisson /( (1+Poisson) * (1-2*Poisson) );
    
    /* Assembly: loop over the elements */
    int ie;
    
#pragma omp parallel for shared(invjac,detjac,elements,myRrows,myRcoef,myAcols,myArows,myAcoef,U_h) private(gradphi,F,EPS,dP,P_Uh,GradV,GradU,GradUh,ie,k,l,q,d1,d2) firstprivate(phi,gradrefphi,w,numRowsElements,nln2,nln,NumNodes,Id,mu,lambda)
    
    for (ie = 0; ie < noe; ie = ie + 1 )
    {
        for (k = 0; k < nln; k = k + 1 )
        {
            for (q = 0; q < NumQuadPoints; q = q + 1 )
            {
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    gradphi[d1][k][q] = 0;
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        gradphi[d1][k][q] = gradphi[d1][k][q] + INVJAC(ie,d1,d2)*GRADREFPHI(k,q,d2);
                    }
                }
            }
        }
        
        for (q = 0; q < NumQuadPoints; q = q + 1 )
        {
            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
            {
                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                {
                    GradUh[d1][d2][q] = 0;
                    for (k = 0; k < nln; k = k + 1 )
                    {
                        int e_k;
                        e_k = (int)(elements[ie*numRowsElements + k] + d1*NumNodes - 1);
                        GradUh[d1][d2][q] = GradUh[d1][d2][q] + U_h[e_k] * gradphi[d2][k][q];
                    }
                }
            }
        }
        
        int iii = 0;
        int ii = 0;
        int a, b, i_c, j_c;
        
        /* loop over test functions --> a */
        for (a = 0; a < nln; a = a + 1 )
        {
            /* loop over test components --> i_c */
            for (i_c = 0; i_c < dim; i_c = i_c + 1 )
            {
                /* set gradV to zero*/
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[d1][d2] = 0;
                    }
                }
                
                /* loop over trial functions --> b */
                for (b = 0; b < nln; b = b + 1 )
                {
                    /* loop over trial components --> j_c */
                    for (j_c = 0; j_c < dim; j_c = j_c + 1 )
                    {
                        /* set gradU to zero*/
                        for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradU[d1][d2] = 0;
                            }
                        }
                        
                        double aloc = 0;
                        for (q = 0; q < NumQuadPoints; q = q + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradV[i_c][d2] = gradphi[d2][a][q];
                                GradU[j_c][d2] = gradphi[d2][b][q];
                            }
                            
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    F[d1][d2] = Id[d1][d2] + GradU[d1][d2];
                                }
                            }
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    EPS[d1][d2] = 0.5 * ( F[d1][d2] + F[d2][d1] ) - Id[d1][d2];
                                }
                            }
                            
                            
                            double trace = Trace(dim, EPS);
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    dP[d1][d2] = 2 * mu * EPS[d1][d2] + lambda * trace * Id[d1][d2];
                                }
                            }
                            aloc  = aloc + Mdot( dim, GradV, dP) * w[q];
                        }
                        myArows[ie*nln2*dim*dim+iii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                        myAcols[ie*nln2*dim*dim+iii] = elements[b+ie*numRowsElements] + j_c * NumNodes;
                        myAcoef[ie*nln2*dim*dim+iii] = aloc*detjac[ie];
                        
                        iii = iii + 1;
                    }
                }
                
                double rloc = 0;
                for (q = 0; q < NumQuadPoints; q = q + 1 )
                {
                    
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[i_c][d2] = gradphi[d2][a][q];
                    }
                    
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            F[d1][d2] = Id[d1][d2] + GradUh[d1][d2][q];
                        }
                    }
                    
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            EPS[d1][d2] = 0.5 * ( F[d1][d2] + F[d2][d1] ) - Id[d1][d2];
                        }
                    }
                    
                    double trace = Trace(dim, EPS);
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            P_Uh[d1][d2] = 2 * mu * EPS[d1][d2] + lambda * trace * Id[d1][d2];
                        }
                    }
                    rloc  = rloc + Mdot( dim, GradV, P_Uh) * w[q];
                }
                                            
                myRrows[ie*nln*dim+ii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                myRcoef[ie*nln*dim+ii] = rloc*detjac[ie];
                ii = ii + 1;
            }
        }
    }
        
}
/*************************************************************************/
void SEMMTMaterial(mxArray* plhs[], const mxArray* prhs[])
{
    
    double* dim_ptr = mxGetPr(prhs[0]);
    int dim     = (int)(dim_ptr[0]);
    int noe     = mxGetN(prhs[4]);
    double* nln_ptr = mxGetPr(prhs[5]);
    int nln     = (int)(nln_ptr[0]);
    int numRowsElements  = mxGetM(prhs[4]);
    int nln2    = nln*nln;
    
    plhs[0] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[2] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[3] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    plhs[4] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    
    double* myArows    = mxGetPr(plhs[0]);
    double* myAcols    = mxGetPr(plhs[1]);
    double* myAcoef    = mxGetPr(plhs[2]);
    double* myRrows    = mxGetPr(plhs[3]);
    double* myRcoef    = mxGetPr(plhs[4]);
    
    int k,l;
    int q;
    int NumQuadPoints     = mxGetN(prhs[6]);
    int NumNodes          = (int)(mxGetM(prhs[3]) / dim);
    
    double* U_h   = mxGetPr(prhs[3]);
    double* w   = mxGetPr(prhs[6]);
    double* invjac = mxGetPr(prhs[7]);
    double* detjac = mxGetPr(prhs[8]);
    double* phi = mxGetPr(prhs[9]);
    double* gradrefphi = mxGetPr(prhs[10]);
    
    double gradphi[dim][nln][NumQuadPoints];
    double* elements  = mxGetPr(prhs[4]);
    
    double GradV[dim][dim];
    double GradU[dim][dim];
    double GradUh[dim][dim][NumQuadPoints];
    
    double Id[dim][dim];
    int d1,d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            Id[d1][d2] = 0;
            if (d1==d2)
            {
                Id[d1][d2] = 1;
            }
        }
    }
    
    double F[dim][dim];
    double EPS[dim][dim];
    double dP[dim][dim];
    double P_Uh[dim][dim];
    
    double* material_param = mxGetPr(prhs[2]);
    double Young = material_param[0];
    double Poisson = material_param[1];
    double Stiffening_power = material_param[2];
    double mu = Young / (2 + 2 * Poisson);
    double lambda =  Young * Poisson /( (1+Poisson) * (1-2*Poisson) );
    
    /* Assembly: loop over the elements */
    int ie;
    
#pragma omp parallel for shared(invjac,detjac,elements,myRrows,myRcoef,myAcols,myArows,myAcoef,U_h) private(gradphi,F,EPS,dP,P_Uh,GradV,GradU,GradUh,ie,k,l,q,d1,d2) firstprivate(phi,gradrefphi,w,numRowsElements,nln2,nln,NumNodes,Id,mu,lambda)
    
    for (ie = 0; ie < noe; ie = ie + 1 )
    {
        for (k = 0; k < nln; k = k + 1 )
        {
            for (q = 0; q < NumQuadPoints; q = q + 1 )
            {
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    gradphi[d1][k][q] = 0;
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        gradphi[d1][k][q] = gradphi[d1][k][q] + INVJAC(ie,d1,d2)*GRADREFPHI(k,q,d2);
                    }
                }
            }
        }
        
        for (q = 0; q < NumQuadPoints; q = q + 1 )
        {
            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
            {
                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                {
                    GradUh[d1][d2][q] = 0;
                    for (k = 0; k < nln; k = k + 1 )
                    {
                        int e_k;
                        e_k = (int)(elements[ie*numRowsElements + k] + d1*NumNodes - 1);
                        GradUh[d1][d2][q] = GradUh[d1][d2][q] + U_h[e_k] * gradphi[d2][k][q];
                    }
                }
            }
        }
        
        int iii = 0;
        int ii = 0;
        int a, b, i_c, j_c;
        
        /* loop over test functions --> a */
        for (a = 0; a < nln; a = a + 1 )
        {
            /* loop over test components --> i_c */
            for (i_c = 0; i_c < dim; i_c = i_c + 1 )
            {
                /* set gradV to zero*/
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[d1][d2] = 0;
                    }
                }
                
                /* loop over trial functions --> b */
                for (b = 0; b < nln; b = b + 1 )
                {
                    /* loop over trial components --> j_c */
                    for (j_c = 0; j_c < dim; j_c = j_c + 1 )
                    {
                        /* set gradU to zero*/
                        for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradU[d1][d2] = 0;
                            }
                        }
                        
                        double aloc = 0;
                        for (q = 0; q < NumQuadPoints; q = q + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradV[i_c][d2] = gradphi[d2][a][q];
                                GradU[j_c][d2] = gradphi[d2][b][q];
                            }
                            
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    F[d1][d2] = Id[d1][d2] + GradU[d1][d2];
                                }
                            }
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    EPS[d1][d2] = 0.5 * ( F[d1][d2] + F[d2][d1] ) - Id[d1][d2];
                                }
                            }
                            
                            
                            double trace = Trace(dim, EPS);
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    dP[d1][d2] = 2 * mu * EPS[d1][d2] + lambda * trace * Id[d1][d2];
                                }
                            }
                            aloc  = aloc + Mdot( dim, GradV, dP) * w[q];
                        }
                        myArows[ie*nln2*dim*dim+iii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                        myAcols[ie*nln2*dim*dim+iii] = elements[b+ie*numRowsElements] + j_c * NumNodes;
                        myAcoef[ie*nln2*dim*dim+iii] = aloc * detjac[ie] * pow( detjac[0] / detjac[ie], Stiffening_power );
                        
                        iii = iii + 1;
                    }
                }
                
                double rloc = 0;
                for (q = 0; q < NumQuadPoints; q = q + 1 )
                {
                    
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[i_c][d2] = gradphi[d2][a][q];
                    }
                    
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            F[d1][d2] = Id[d1][d2] + GradUh[d1][d2][q];
                        }
                    }
                    
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            EPS[d1][d2] = 0.5 * ( F[d1][d2] + F[d2][d1] ) - Id[d1][d2];
                        }
                    }
                    
                    double trace = Trace(dim, EPS);
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            P_Uh[d1][d2] = 2 * mu * EPS[d1][d2] + lambda * trace * Id[d1][d2];
                        }
                    }
                    rloc  = rloc + Mdot( dim, GradV, P_Uh) * w[q];
                }
                                            
                myRrows[ie*nln*dim+ii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                myRcoef[ie*nln*dim+ii] = rloc * detjac[ie] * pow( detjac[0] / detjac[ie], Stiffening_power );
                ii = ii + 1;
            }
        }
    }
        
}
/*************************************************************************/

void StVenantKirchhoffMaterial(mxArray* plhs[], const mxArray* prhs[])
{
    
    double* dim_ptr = mxGetPr(prhs[0]);
    int dim     = (int)(dim_ptr[0]);
    int noe     = mxGetN(prhs[4]);
    double* nln_ptr = mxGetPr(prhs[5]);
    int nln     = (int)(nln_ptr[0]);
    int numRowsElements  = mxGetM(prhs[4]);
    int nln2    = nln*nln;
    
    plhs[0] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[2] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[3] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    plhs[4] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    
    double* myArows    = mxGetPr(plhs[0]);
    double* myAcols    = mxGetPr(plhs[1]);
    double* myAcoef    = mxGetPr(plhs[2]);
    double* myRrows    = mxGetPr(plhs[3]);
    double* myRcoef    = mxGetPr(plhs[4]);
    
    int k,l;
    int q;
    int NumQuadPoints     = mxGetN(prhs[6]);
    int NumNodes          = (int)(mxGetM(prhs[3]) / dim);
    
    double* U_h   = mxGetPr(prhs[3]);
    double* w   = mxGetPr(prhs[6]);
    double* invjac = mxGetPr(prhs[7]);
    double* detjac = mxGetPr(prhs[8]);
    double* phi = mxGetPr(prhs[9]);
    double* gradrefphi = mxGetPr(prhs[10]);
    
    double gradphi[dim][nln][NumQuadPoints];
    double* elements  = mxGetPr(prhs[4]);
    
    double GradV[dim][dim];
    double GradU[dim][dim];
    double GradUh[dim][dim][NumQuadPoints];
    
    double Id[dim][dim];
    int d1,d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            Id[d1][d2] = 0;
            if (d1==d2)
            {
                Id[d1][d2] = 1;
            }
        }
    }
    
    double F[dim][dim][NumQuadPoints];
    double E[dim][dim][NumQuadPoints];
    double dP[dim][dim];
    double P_Uh[dim][dim];
    
    double dF[dim][dim];
    double dE[dim][dim];
    
    double* material_param = mxGetPr(prhs[2]);
    double Young = material_param[0];
    double Poisson = material_param[1];
    double mu = Young / (2 + 2 * Poisson);
    double lambda =  Young * Poisson /( (1+Poisson) * (1-2*Poisson) );
    
    /* Assembly: loop over the elements */
    int ie;
    
#pragma omp parallel for shared(invjac,detjac,elements,myRrows,myRcoef,myAcols,myArows,myAcoef,U_h) private(gradphi,F,E,dP,P_Uh,dF,dE,GradV,GradU,GradUh,ie,k,l,q,d1,d2) firstprivate(phi,gradrefphi,w,numRowsElements,nln2,nln,NumNodes,Id,mu,lambda)
    
    for (ie = 0; ie < noe; ie = ie + 1 )
    {             
        double traceE[NumQuadPoints];
        for (q = 0; q < NumQuadPoints; q = q + 1 )
        {
            /* Compute Gradient of Basis functions*/
            for (k = 0; k < nln; k = k + 1 )
            {
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    gradphi[d1][k][q] = 0;
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        gradphi[d1][k][q] = gradphi[d1][k][q] + INVJAC(ie,d1,d2)*GRADREFPHI(k,q,d2);
                    }
                }
            }
            
            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
            {
                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                {
                    GradUh[d1][d2][q] = 0;
                    for (k = 0; k < nln; k = k + 1 )
                    {
                        int e_k;
                        e_k = (int)(elements[ie*numRowsElements + k] + d1*NumNodes - 1);
                        GradUh[d1][d2][q] = GradUh[d1][d2][q] + U_h[e_k] * gradphi[d2][k][q];
                    }
                    F[d1][d2][q]  = Id[d1][d2] + GradUh[d1][d2][q];
                }
            }
            compute_GreenStrainTensor(dim, NumQuadPoints, F, Id, E, q );
            traceE[q] = TraceQ(dim, NumQuadPoints, E, q);
        }

        int iii = 0;
        int ii = 0;
        int a, b, i_c, j_c;
        
        /* loop over test functions --> a */
        for (a = 0; a < nln; a = a + 1 )
        {
            /* loop over test components --> i_c */
            for (i_c = 0; i_c < dim; i_c = i_c + 1 )
            {
                /* set gradV to zero*/
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[d1][d2] = 0;
                    }
                }
                
                /* loop over trial functions --> b */
                for (b = 0; b < nln; b = b + 1 )
                {
                    /* loop over trial components --> j_c */
                    for (j_c = 0; j_c < dim; j_c = j_c + 1 )
                    {
                        /* set gradU to zero*/
                        for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradU[d1][d2] = 0;
                            }
                        }
                        
                        double aloc = 0;
                        for (q = 0; q < NumQuadPoints; q = q + 1 )
                        {
                            
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradV[i_c][d2] = gradphi[d2][a][q];
                                GradU[j_c][d2] = gradphi[d2][b][q];
                            }
                            
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    dF[d1][d2] = GradU[d1][d2];
                                }
                            }
                            
                            compute_DerGreenStrainTensor(dim, NumQuadPoints, F, dF, dE, q );
                            
                            double trace_dE = Trace(dim, dE);
                            double P1[dim][dim];
                            double P2[dim][dim];
                            double P_tmp[dim][dim];
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    P1[d1][d2] =  2 * mu * E[d1][d2][q]  + lambda * traceE[q]  * Id[d1][d2] ;
                                    P2[d1][d2] =  2 * mu * dE[d1][d2] + lambda * trace_dE * Id[d1][d2] ;                                    
                                }
                            }
                            MatrixProduct(dim, dF, P1, dP);
                            MatrixProductQ1(dim, NumQuadPoints, F, P2,  P_tmp, q);
                            MatrixSum(dim, dP, P_tmp);
                            aloc  = aloc + Mdot( dim, GradV, dP) * w[q];
                        }
                        myArows[ie*nln2*dim*dim+iii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                        myAcols[ie*nln2*dim*dim+iii] = elements[b+ie*numRowsElements] + j_c * NumNodes;
                        myAcoef[ie*nln2*dim*dim+iii] = aloc*detjac[ie];
                        
                        iii = iii + 1;
                    }
                }
                
                double rloc = 0;
                for (q = 0; q < NumQuadPoints; q = q + 1 )
                {
                    
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[i_c][d2] = gradphi[d2][a][q];
                    }
                    
                    double P1[dim][dim];
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            P1[d1][d2] =  ( 2 * mu * E[d1][d2][q] + lambda * traceE[q] * Id[d1][d2] );
                        }
                    }
                    
                    MatrixProductQ1(dim, NumQuadPoints, F, P1, P_Uh, q);  
                    rloc  = rloc + Mdot( dim, GradV, P_Uh) * w[q];
                }
                                            
                myRrows[ie*nln*dim+ii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                myRcoef[ie*nln*dim+ii] = rloc*detjac[ie];
                ii = ii + 1;
            }
        }
    }
        
}

/*************************************************************************/

void NeoHookeanMaterial(mxArray* plhs[], const mxArray* prhs[])
{
    
    double* dim_ptr = mxGetPr(prhs[0]);
    int dim     = (int)(dim_ptr[0]);
    int noe     = mxGetN(prhs[4]);
    double* nln_ptr = mxGetPr(prhs[5]);
    int nln     = (int)(nln_ptr[0]);
    int numRowsElements  = mxGetM(prhs[4]);
    int nln2    = nln*nln;
    
    plhs[0] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[2] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[3] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    plhs[4] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    
    double* myArows    = mxGetPr(plhs[0]);
    double* myAcols    = mxGetPr(plhs[1]);
    double* myAcoef    = mxGetPr(plhs[2]);
    double* myRrows    = mxGetPr(plhs[3]);
    double* myRcoef    = mxGetPr(plhs[4]);
    
    int k,l;
    int q;
    int NumQuadPoints     = mxGetN(prhs[6]);
    int NumNodes          = (int)(mxGetM(prhs[3]) / dim);
    
    double* U_h   = mxGetPr(prhs[3]);
    double* w   = mxGetPr(prhs[6]);
    double* invjac = mxGetPr(prhs[7]);
    double* detjac = mxGetPr(prhs[8]);
    double* phi = mxGetPr(prhs[9]);
    double* gradrefphi = mxGetPr(prhs[10]);
    
    double gradphi[dim][nln][NumQuadPoints];
    double* elements  = mxGetPr(prhs[4]);
    
    double GradV[dim][dim];
    double GradU[dim][dim];
    double GradUh[dim][dim][NumQuadPoints];
    
    double Id[dim][dim];
    int d1,d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            Id[d1][d2] = 0;
            if (d1==d2)
            {
                Id[d1][d2] = 1;
            }
        }
    }
    
    double F[NumQuadPoints][dim][dim];
    double invFT[NumQuadPoints][dim][dim];
    double detF[NumQuadPoints];
    double logdetF[NumQuadPoints];
    double dP[dim][dim];
    double P_Uh[dim][dim];
    
    double dF[dim][dim];
    double dE[dim][dim];
    
    double* material_param = mxGetPr(prhs[2]);
    double Young = material_param[0];
    double Poisson = material_param[1];
    double mu = Young / (2 + 2 * Poisson);
    double lambda =  Young * Poisson /( (1+Poisson) * (1-2*Poisson) );
    
    /* Assembly: loop over the elements */
    int ie;
    
#pragma omp parallel for shared(invjac,detjac,elements,myRrows,myRcoef,myAcols,myArows,myAcoef,U_h) private(gradphi,F,invFT,logdetF,detF,dP,P_Uh,dF,GradV,GradU,GradUh,ie,k,l,q,d1,d2) firstprivate(phi,gradrefphi,w,numRowsElements,nln2,nln,NumNodes,Id,mu,lambda)
    
    for (ie = 0; ie < noe; ie = ie + 1 )
    {             
        for (q = 0; q < NumQuadPoints; q = q + 1 )
        {
            /* Compute Gradient of Basis functions*/
            for (k = 0; k < nln; k = k + 1 )
            {
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    gradphi[d1][k][q] = 0;
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        gradphi[d1][k][q] = gradphi[d1][k][q] + INVJAC(ie,d1,d2)*GRADREFPHI(k,q,d2);
                    }
                }
            }
            
            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
            {
                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                {
                    GradUh[d1][d2][q] = 0;
                    for (k = 0; k < nln; k = k + 1 )
                    {
                        int e_k;
                        e_k = (int)(elements[ie*numRowsElements + k] + d1*NumNodes - 1);
                        GradUh[d1][d2][q] = GradUh[d1][d2][q] + U_h[e_k] * gradphi[d2][k][q];
                    }
                    F[q][d1][d2]  = Id[d1][d2] + GradUh[d1][d2][q];
                }
            }
            detF[q] = MatrixDeterminant(dim, F[q]);
            MatrixInvT(dim, F[q], invFT[q] );
            logdetF[q] = log( detF[q] );
        }

        int iii = 0;
        int ii = 0;
        int a, b, i_c, j_c;
        
        /* loop over test functions --> a */
        for (a = 0; a < nln; a = a + 1 )
        {
            /* loop over test components --> i_c */
            for (i_c = 0; i_c < dim; i_c = i_c + 1 )
            {
                /* set gradV to zero*/
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[d1][d2] = 0;
                    }
                }
                
                /* loop over trial functions --> b */
                for (b = 0; b < nln; b = b + 1 )
                {
                    /* loop over trial components --> j_c */
                    for (j_c = 0; j_c < dim; j_c = j_c + 1 )
                    {
                        /* set gradU to zero*/
                        for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradU[d1][d2] = 0;
                            }
                        }
                        
                        double aloc = 0;
                        for (q = 0; q < NumQuadPoints; q = q + 1 )
                        {
                            
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradV[i_c][d2] = gradphi[d2][a][q];
                                GradU[j_c][d2] = gradphi[d2][b][q];
                            }
                            
                            
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    dF[d1][d2] = GradU[d1][d2];
                                }
                            }
                                                        
                            double P2[dim][dim];
                            double P3[dim][dim];
                            double P_tmp[dim][dim];
                            double P_tmp2[dim][dim];
                            
                            MatrixProductAlphaT2(dim, mu-lambda*logdetF[q], invFT[q], dF, P_tmp);
                            MatrixProductAlpha(dim, 1.0, P_tmp, invFT[q], dP);
                            
                            MatrixProductAlphaT1( dim, 1.0, invFT[q], dF, P_tmp2);
                            double coef = lambda * Trace(dim, P_tmp2);
                            
                            MatrixProductAlpha(dim, coef, Id, invFT[q], P2 );
                            
                            MatrixSum(dim, dP, P2);
                            
                            MatrixProductAlpha(dim, mu, Id, dF, P3 );
                            
                            MatrixSum(dim, dP, P3);
                            
                            aloc  = aloc + Mdot( dim, GradV, dP) * w[q];
                        }
                        myArows[ie*nln2*dim*dim+iii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                        myAcols[ie*nln2*dim*dim+iii] = elements[b+ie*numRowsElements] + j_c * NumNodes;
                        myAcoef[ie*nln2*dim*dim+iii] = aloc*detjac[ie];
                        
                        iii = iii + 1;
                    }
                }
                
                double rloc = 0;
                for (q = 0; q < NumQuadPoints; q = q + 1 )
                {
                    
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[i_c][d2] = gradphi[d2][a][q];
                    }
                    
                    double P1[dim][dim];
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {
                            P_Uh[d1][d2] =  ( mu * ( F[q][d1][d2] - invFT[q][d1][d2] ) + lambda * logdetF[q] * invFT[q][d1][d2] );
                        }
                    }
                    rloc  = rloc + Mdot( dim, GradV, P_Uh) * w[q];
                }
                                            
                myRrows[ie*nln*dim+ii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                myRcoef[ie*nln*dim+ii] = rloc*detjac[ie];
                ii = ii + 1;
            }
        }
    }
        
}
/*************************************************************************/

void NeoHookean2Material(mxArray* plhs[], const mxArray* prhs[])
{
    
    double* dim_ptr = mxGetPr(prhs[0]);
    int dim     = (int)(dim_ptr[0]);
    int noe     = mxGetN(prhs[4]);
    double* nln_ptr = mxGetPr(prhs[5]);
    int nln     = (int)(nln_ptr[0]);
    int numRowsElements  = mxGetM(prhs[4]);
    int nln2    = nln*nln;
    
    plhs[0] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[1] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[2] = mxCreateDoubleMatrix(nln2*noe*dim*dim,1, mxREAL);
    plhs[3] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    plhs[4] = mxCreateDoubleMatrix(nln*noe*dim,1, mxREAL);
    
    double* myArows    = mxGetPr(plhs[0]);
    double* myAcols    = mxGetPr(plhs[1]);
    double* myAcoef    = mxGetPr(plhs[2]);
    double* myRrows    = mxGetPr(plhs[3]);
    double* myRcoef    = mxGetPr(plhs[4]);
    
    int k,l;
    int q;
    int NumQuadPoints     = mxGetN(prhs[6]);
    int NumNodes          = (int)(mxGetM(prhs[3]) / dim);
    
    double* U_h   = mxGetPr(prhs[3]);
    double* w   = mxGetPr(prhs[6]);
    double* invjac = mxGetPr(prhs[7]);
    double* detjac = mxGetPr(prhs[8]);
    double* phi = mxGetPr(prhs[9]);
    double* gradrefphi = mxGetPr(prhs[10]);
    
    double* elements  = mxGetPr(prhs[4]);
    
    double Id[dim][dim];
    int d1,d2;
    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
    {
        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
        {
            Id[d1][d2] = 0;
            if (d1==d2)
            {
                Id[d1][d2] = 1;
            }
        }
    }
    
    double* material_param = mxGetPr(prhs[2]);
    double Young = material_param[0];
    double Poisson = material_param[1];
    double mu = Young / (2.0 + 2.0 * Poisson);
    double lambda =  Young * Poisson /( (1.0 + Poisson) * (1.0-2.0*Poisson) );
    double bulk = ( 2.0 / 3.0 ) * mu + lambda;
    
    /* Assembly: loop over the elements */
    int ie;
    
#pragma omp parallel for shared(invjac,detjac,elements,myRrows,myRcoef,myAcols,myArows,myAcoef,U_h) private(ie,k,l,q,d1,d2) firstprivate(phi,gradrefphi,w,numRowsElements,nln2,nln,NumNodes,Id,mu,lambda)
    
    for (ie = 0; ie < noe; ie = ie + 1 )
    {             
        double I_C[NumQuadPoints];
        double detF[NumQuadPoints];
        double logdetF[NumQuadPoints];
        double pow23detF[NumQuadPoints];
        double pow2detF[NumQuadPoints];
        
        double F[NumQuadPoints][dim][dim];
        double invFT[NumQuadPoints][dim][dim];
        double C[NumQuadPoints][dim][dim];
        
        double dP[dim][dim];
        double P_Uh[dim][dim];
        
        double GradV[dim][dim];
        double GradU[dim][dim];
        double GradUh[NumQuadPoints][dim][dim];
        
        double gradphi[dim][nln][NumQuadPoints];

        for (q = 0; q < NumQuadPoints; q = q + 1 )
        {
            /* Compute Gradient of Basis functions*/
            for (k = 0; k < nln; k = k + 1 )
            {
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    gradphi[d1][k][q] = 0;
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        gradphi[d1][k][q] = gradphi[d1][k][q] + INVJAC(ie,d1,d2)*GRADREFPHI(k,q,d2);
                    }
                }
            }
            
            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
            {
                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                {
                    GradUh[q][d1][d2] = 0;
                    for (k = 0; k < nln; k = k + 1 )
                    {
                        int e_k;
                        e_k = (int)(elements[ie*numRowsElements + k] + d1*NumNodes - 1);
                        GradUh[q][d1][d2] = GradUh[q][d1][d2] + U_h[e_k] * gradphi[d2][k][q];
                    }
                    F[q][d1][d2]  = Id[d1][d2] + GradUh[q][d1][d2];
                }
            }
            detF[q] = MatrixDeterminant(dim, F[q]);
            MatrixInvT(dim, F[q], invFT[q] );
            MatrixProductAlphaT1(dim, 1.0, F[q], F[q], C[q] );
            logdetF[q] = log( detF[q] );
            pow23detF[q] = pow(detF[q], -2.0 / 3.0);
            pow2detF[q] = pow(detF[q], 2.0);
            I_C[q] = Trace(dim, C[q]);
        }

        int iii = 0;
        int ii = 0;
        int a, b, i_c, j_c;
        
        /* loop over test functions --> a */
        for (a = 0; a < nln; a = a + 1 )
        {
            /* loop over test components --> i_c */
            for (i_c = 0; i_c < dim; i_c = i_c + 1 )
            {
                /* set gradV to zero*/
                for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                {
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[d1][d2] = 0;
                    }
                }
                
                /* loop over trial functions --> b */
                for (b = 0; b < nln; b = b + 1 )
                {
                    /* loop over trial components --> j_c */
                    for (j_c = 0; j_c < dim; j_c = j_c + 1 )
                    {
                        /* set gradU to zero*/
                        for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                        {
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradU[d1][d2] = 0;
                            }
                        }
                        
                        double aloc = 0;
                        for (q = 0; q < NumQuadPoints; q = q + 1 )
                        {
                            
                            for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                            {
                                GradV[i_c][d2] = gradphi[d2][a][q];
                                GradU[j_c][d2] = gradphi[d2][b][q];
                            }
                            
                            /* volumetric part */
                            double dP_vol[dim][dim];
                            double dP_vol1[dim][dim];
                            double dP_vol2_tmp[dim][dim];
                            double dP_vol2[dim][dim];
                            
                            MatrixScalar(dim, 0.5*bulk * (2.0*pow2detF[q] -detF[q] + 1.0)*Mdot(dim, invFT[q], GradU),
                                    invFT[q], dP_vol);
                            
                            MatrixProductAlphaT2(dim, 0.5*bulk * ( - pow2detF[q] + detF[q] - logdetF[q]), invFT[q], GradU, dP_vol2_tmp);
                            MatrixProductAlpha(dim, 1.0, dP_vol2_tmp, invFT[q], dP_vol2);
                            
                            MatrixSum(dim, dP_vol, dP_vol2);
                            
                            /* isochoric part */
                            double dP_iso[dim][dim];
                            double dP_iso1[dim][dim];
                            double dP_iso24[dim][dim];
                            /*double dP_iso2[dim][dim];*/
                            double dP_iso3[dim][dim];
                            /*double dP_iso4[dim][dim];*/
                            double dP_iso5[dim][dim];
                            double dP_iso5_tmp[dim][dim];
                            double dP_iso5_tmp2[dim][dim];
                            
                            MatrixScalar(dim, -2.0 / 3.0 * mu * pow23detF[q] * Mdot(dim, invFT[q], GradU),
                                    F[q], dP_iso1);
                            
                            /*MatrixScalar(dim, 2.0 / 9.0 * mu * pow23detF[q] * Trace(dim, C[q]) * Mdot(dim, invFT[q], GradU),
                                    invFT[q], dP_iso2);*/
                            MatrixScalar(dim, mu * pow23detF[q] * 
                                                ( 2.0 / 9.0 * I_C[q]  * Mdot(dim, invFT[q], GradU) 
                                                 -2.0 / 3.0 * Mdot(dim, F[q], GradU) ),
                                    invFT[q], dP_iso24);
                            
                            MatrixScalar(dim, mu * pow23detF[q], GradU, dP_iso3);
                            
                            /*MatrixScalar(dim, -2.0 / 3.0 * mu * pow23detF[q]* Mdot(dim, F[q], GradU), invFT[q], dP_iso4);*/
                            
                            MatrixProductAlphaT2(dim, 1.0, invFT[q], GradU, dP_iso5_tmp);
                            MatrixProductAlpha(dim, 1.0, dP_iso5_tmp, invFT[q], dP_iso5_tmp2);
                            MatrixScalar(dim, 1.0 / 3.0 * mu * pow23detF[q] * I_C[q] , dP_iso5_tmp2, dP_iso5);
                            
                            /* Sum all contributes */
                            for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                            {
                                for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                                {
                                    dP[d1][d2] = dP_vol[d1][d2] 
                                                + dP_iso1[d1][d2] 
                                                + dP_iso24[d1][d2]
                                                + dP_iso3[d1][d2] 
                                                + dP_iso5[d1][d2];
                                }
                            }
                            aloc  = aloc + Mdot( dim, GradV, dP) * w[q];
                        }
                        myArows[ie*nln2*dim*dim+iii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                        myAcols[ie*nln2*dim*dim+iii] = elements[b+ie*numRowsElements] + j_c * NumNodes;
                        myAcoef[ie*nln2*dim*dim+iii] = aloc*detjac[ie];
                        
                        iii = iii + 1;
                    }
                }
                
                double rloc = 0;
                for (q = 0; q < NumQuadPoints; q = q + 1 )
                {
                    
                    for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                    {
                        GradV[i_c][d2] = gradphi[d2][a][q];
                    }
                    
                    double P1[dim][dim];
                    for (d1 = 0; d1 < dim; d1 = d1 + 1 )
                    {
                        for (d2 = 0; d2 < dim; d2 = d2 + 1 )
                        {                            
                            P_Uh[d1][d2] =  mu * pow23detF[q] * ( F[q][d1][d2] - 1.0 / 3.0 * I_C[q]  * invFT[q][d1][d2] ) 
                                            + 1.0 / 2.0 * bulk * ( pow2detF[q] - detF[q] + logdetF[q] ) * invFT[q][d1][d2];                            
                        }
                    }
                    rloc  = rloc + Mdot( dim, GradV, P_Uh) * w[q];
                }
                                            
                myRrows[ie*nln*dim+ii] = elements[a+ie*numRowsElements] + i_c * NumNodes;
                myRcoef[ie*nln*dim+ii] = rloc*detjac[ie];
                ii = ii + 1;
            }
        }
    }
        
}
/*************************************************************************/

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    
    /* Check for proper number of arguments. */
    if(nrhs!=11) {
        mexErrMsgTxt("11 inputs are required.");
    } else if(nlhs>5) {
        mexErrMsgTxt("Too many output arguments.");
    }
    
    char *Material_Model = mxArrayToString(prhs[1]);
    
    if (strcmp(Material_Model, "Linear")==0)
    {
            LinearElasticMaterial(plhs, prhs);
    }
    
    if (strcmp(Material_Model, "SEMMT")==0)
    {
            SEMMTMaterial(plhs, prhs);
    }
    
    if (strcmp(Material_Model, "StVenantKirchhoff")==0)
    {
            StVenantKirchhoffMaterial(plhs, prhs);
    }
    
    if (strcmp(Material_Model, "NeoHookean")==0)
    {
            NeoHookeanMaterial(plhs, prhs);
    }
    
    if (strcmp(Material_Model, "NeoHookean2")==0)
    {
            NeoHookean2Material(plhs, prhs);
    }
    
    mxFree(Material_Model);
}
/*************************************************************************/

