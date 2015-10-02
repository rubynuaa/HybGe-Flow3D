#include <iostream>
#include <cstdlib>
#include <vector>
#include <paralution.hpp>

#include "hgfMesh.hpp"
#include "hgf.hpp"
#include "hgfArrays.hpp"
#include "hgfBC.hpp"
#include "hgfIB.hpp"
#include "hgfPP.hpp"

#define idx2(i, j, ldi) ((i * ldi) + j)

using namespace paralution;

void
computeKTensorL ( const FluidMesh& Mesh, \
                  const paralution::LocalVector<double>& xSolution, \
                  const paralution::LocalVector<double>& ySolution, \
                  const paralution::LocalVector<double>& zSolution )
{
  switch ( Mesh.DIM )
  {
    case 3 :
    {
      double GVals[27], Vels[9];
      int GIs[27], GJs[27];

      // Assign Indices
      for (int i = 0; i < 3; i++) {
        // First block
        GIs[i] = 0;
        GJs[i] = i;
        GIs[3+i] = 1;
        GJs[3+i] = i;
        GIs[6+i] = 2;
        GJs[6+i] = i;
        // Second block
        GIs[9+i] = 3;
        GJs[9+i] = 3 + i;
        GIs[12+i] = 4;
        GJs[12+i] = 3 + i;
        GIs[15+i] = 5;
        GJs[15+i] = 3 + i;
        // Third block
        GIs[18+i] = 6;
        GJs[18+i] = 6 + i;
        GIs[21+i] = 7;
        GJs[21+i] = 6 + i;
        GIs[24+i] = 8;
        GJs[24+i] = 6 + i;
      }

      // Compute averages
      computeAveragesX ( Mesh, xSolution, Vels[0], GVals[0] );
      computeAveragesY ( Mesh, xSolution, Vels[1], GVals[1] );
      computeAveragesZ ( Mesh, xSolution, Vels[2], GVals[2] );
      computeAveragesX ( Mesh, ySolution, Vels[3], GVals[3] );
      computeAveragesY ( Mesh, ySolution, Vels[4], GVals[4] );
      computeAveragesZ ( Mesh, ySolution, Vels[5], GVals[5] );
      computeAveragesX ( Mesh, zSolution, Vels[6], GVals[6] );
      computeAveragesY ( Mesh, zSolution, Vels[7], GVals[7] );
      computeAveragesZ ( Mesh, zSolution, Vels[8], GVals[8] );

      for (int i = 0; i < 9; i++) {
        GVals[i+9] = GVals[i];
        GVals[i+18] = GVals[i];
      }

      // Initialize paralution arrays
      LocalVector<double> K;
      LocalVector<double> MacroVels;
      LocalMatrix<double> MacroPresGrads;

      // Conductivity tensor
      K.Allocate("conductivity tensor", 9);
      K.Zeros();

      // Macro velocities
      MacroVels.Allocate("macroscopic velocities", 9);
      MacroVels.Zeros();
      for (int i = 0; i < 9; i++) {
        MacroVels[i] = Vels[i];
      }

      // Pressure gradients matrix
      MacroPresGrads.Assemble(GIs, GJs, GVals, 27, "pressure gradients", 9, 9);

      // Solver setup
      GMRES<LocalMatrix<double>, LocalVector<double>, double > ls;
      ls.SetOperator(MacroPresGrads);
      ls.Verbose(2);
      ls.Build();

      ls.Solve(MacroVels, &K);

      // Write out solution to file
      std::ofstream KTensor;
      KTensor.open ("KTensor.dat");
      KTensor << K[0] << "\t" << K[1] << "\t" << K[2] << "\n";
      KTensor << K[3] << "\t" << K[4] << "\t" << K[5] << "\n";
      KTensor << K[6] << "\t" << K[7] << "\t" << K[8];
      KTensor.close();
      MacroVels.Clear();
      K.Clear();
      MacroPresGrads.Clear();
      break;
    }
    case 2 :
    {
      double GVals[8], Vels[4];
      int GIs[8], GJs[8];

      // Assign Indices
      for (int i = 0; i < 2; i++) {
        // First block
        GIs[i] = 0;
        GJs[i] = i;
        GIs[2+i] = 1;
        GJs[2+i] = i;
        // Second block
        GIs[4+i] = 2;
        GJs[4+i] = 2 + i;
        GIs[6+i] = 3;
        GJs[6+i] = 2 + i;
      }

      // Compute averages
      computeAveragesX ( Mesh, xSolution, Vels[0], GVals[0] );
      computeAveragesY ( Mesh, xSolution, Vels[1], GVals[1] );
      computeAveragesX ( Mesh, ySolution, Vels[3], GVals[3] );
      computeAveragesY ( Mesh, ySolution, Vels[4], GVals[4] );

      for (int i = 0; i < 4; i++) {
        GVals[i+4] = GVals[i];
      }

      // Initialize paralution arrays
      LocalVector<double> K;
      LocalVector<double> MacroVels;
      LocalMatrix<double> MacroPresGrads;

      // Conductivity tensor
      K.Allocate("conductivity tensor", 9);
      K.Zeros();

      // Macro velocities
      MacroVels.Allocate("macroscopic velocities", 9);
      MacroVels.Zeros();
      for (int i = 0; i < 4; i++) {
        MacroVels[i] = Vels[i];
      }

      // Pressure gradients matrix
      MacroPresGrads.Assemble(GIs, GJs, GVals, 8, "pressure gradients", 4, 4);

      // Solver setup
      GMRES<LocalMatrix<double>, LocalVector<double>, double > ls;
      ls.SetOperator(MacroPresGrads);
      ls.Verbose(2);
      ls.Build();

      ls.Solve(MacroVels, &K);

      // Write out solution to file
      std::ofstream KTensor;
      KTensor.open ("KTensor.dat");
      KTensor << K[0] << "\t" << K[1] << "\n";
      KTensor << K[2] << "\t" << K[3] << "\n";
      KTensor.close();
      MacroVels.Clear();
      K.Clear();
      MacroPresGrads.Clear();
      break;
    }
  }
}

void
computeAveragesX ( const FluidMesh& Mesh, \
                   const paralution::LocalVector<double>& Solution, \
                   double& V, double& G )
{
  double xmin, xmax, xmid, midRangex, ymin, ymax, zmin, zmax, K, pNode1, pNode2;
  double P1 = 0;
  double P2 = 0;
  double r = 0.09;
  double pressure;
  int vInd = 0;
  int p1Ind = 0;
  int p2Ind = 0;

  G = 0;
  V = 0;

  switch ( Mesh.DIM )
  {
    case 3 :
    {
      int vDOF = Mesh.DOF[1] + Mesh.DOF[2] + Mesh.DOF[3];
      // Geometric properties for volume averaging
      xmin = Mesh.PCellCenters[ idx2( 0, 0, Mesh.CellCentersLDI ) ];
      xmax = xmin;
      ymin = Mesh.PCellCenters[ idx2( 0, 1, Mesh.CellCentersLDI ) ];
      ymax = ymin;
      zmin = Mesh.PCellCenters[ idx2( 0, 2, Mesh.CellCentersLDI ) ];
      zmax = zmin;

      for (int cl = 0; cl < Mesh.DOF[1]; cl++)
      {
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmin) {
          xmin = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmax) {
          xmax = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
      }
      double L = xmax - xmin;
      for (int cl = 0; cl < Mesh.DOF[2]; cl++)
      {
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymin) {
          ymin = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymax) {
          ymax = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
      }
      double W = ymax - ymin;
      for (int cl = 0; cl < Mesh.DOF[3]; cl++)
      {
        if (Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] < zmin) {
          zmin = Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] > zmax) {
          zmax = Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ];
        }
      }
      double H = zmax - zmin;

      // Adjust min and max to avoid potential recirculation near boundaries.
      // Then compute mid and midRange.
      xmin = xmin + r*L;
      xmax = xmax - r*L;
      xmid = 0.5 * (xmin + xmax);
      zmin = zmin + r*H;
      zmax = zmax - r*H;
      ymin = ymin + r*W;
      ymax = ymax - r*W;
      midRangex = 0.5 * (xmax + xmid) - 0.5 * (xmid + xmin);

      // Compute averages
      for (int cl = 0; cl < Mesh.DOF[1]; cl++)
      {
        pNode1 = Mesh.UCellPressureNeighbor[ idx2( cl, 0, Mesh.VelocityCellPressureNeighborLDI ) ];
        pNode2 = Mesh.UCellPressureNeighbor[ idx2( cl, 1, Mesh.VelocityCellPressureNeighborLDI ) ];
        if (!Mesh.ImmersedBoundary[ pNode1 ] && !Mesh.ImmersedBoundary[ pNode2 ])
        {
          if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmin)
          {
            if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmax) // Inside x limits
            {
              if (Mesh.UCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymin)
              {
                if (Mesh.UCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymax) // Inside y limits
                {
                  if (Mesh.UCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] > zmin)
                  {
                    if (Mesh.UCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] < zmax) // Inside z limits
                    {
                      V = V + Solution[ cl ];
                      vInd++;
                      pressure = 0.5 * (Solution[ pNode1 + vDOF ] + Solution[ pNode2 + vDOF ] );
                      if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmid)
                      {
                        P1 = P1 + pressure;
                        p1Ind++;
                      }
                      else if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmid)
                      {
                        P2 = P2 + pressure;
                        p2Ind++;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      V = V / vInd;
      P1 = P1 / p1Ind;
      P2 = P2 / p2Ind;
      G = (P1 - P2) / midRangex;

      K = V / G;

      // Write out K
      std::ofstream KConstantX;
      KConstantX.open("KConstantX.dat");
      KConstantX << K;
      KConstantX.close();
      break;
    }
    case 2 :
    {
      int vDOF = Mesh.DOF[1] + Mesh.DOF[2];
      // Geometric properties for volume averaging
      xmin = Mesh.PCellCenters[ idx2( 0, 0, Mesh.CellCentersLDI ) ];
      xmax = xmin;
      ymin = Mesh.PCellCenters[ idx2( 0, 1, Mesh.CellCentersLDI ) ];
      ymax = ymin;

      for (int cl = 0; cl < Mesh.DOF[1]; cl++)
      {
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmin) {
          xmin = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmax) {
          xmax = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
      }
      double L = xmax - xmin;
      for (int cl = 0; cl < Mesh.DOF[2]; cl++)
      {
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymin) {
          ymin = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymax) {
          ymax = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
      }
      double W = ymax - ymin;

      // Adjust min and max to avoid potential recirculation near boundaries.
      // Then compute mid and midRange.
      xmin = xmin + r*L;
      xmax = xmax - r*L;
      xmid = 0.5 * (xmin + xmax);
      ymin = ymin + r*W;
      ymax = ymax - r*W;
      midRangex = 0.5 * (xmax + xmid) - 0.5 * (xmid + xmin);

      // Compute averages
      for (int cl = 0; cl < Mesh.DOF[1]; cl++)
      {
        pNode1 = Mesh.UCellPressureNeighbor[ idx2( cl, 0, Mesh.VelocityCellPressureNeighborLDI ) ];
        pNode2 = Mesh.UCellPressureNeighbor[ idx2( cl, 1, Mesh.VelocityCellPressureNeighborLDI ) ];
        if (!Mesh.ImmersedBoundary[ pNode1 ] && !Mesh.ImmersedBoundary[ pNode2 ])
        {
          if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmin)
          {
            if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmax) // Inside x limits
            {
              if (Mesh.UCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymin)
              {
                if (Mesh.UCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymax) // Inside y limits
                {
                  V = V + Solution[ cl ];
                  vInd++;
                  pressure = 0.5 * (Solution[ pNode1 + vDOF ] + Solution[ pNode2 + vDOF ] );
                  if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmid)
                  {
                    P1 = P1 + pressure;
                    p1Ind++;
                  }
                  else if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmid)
                  {
                    P2 = P2 + pressure;
                    p2Ind++;
                  }
                }
              }
            }
          }
        }
      }
      V = V / vInd;
      P1 = P1 / p1Ind;
      P2 = P2 / p2Ind;
      G = (P1 - P2) / midRangex;

      K = V / G;

      // Write out K
      std::ofstream KConstantX;
      KConstantX.open("KConstantX.dat");
      KConstantX << K;
      KConstantX.close();
      break;
    }
  }

}

void
computeAveragesY ( const FluidMesh& Mesh, \
                   const paralution::LocalVector<double>& Solution, \
                   double& V, double& G )
{
  double xmin, xmax, midRangey, ymin, ymax, ymid, zmin, zmax, K, pNode1, pNode2;
  double P1 = 0;
  double P2 = 0;
  double r = 0.09;
  double pressure;
  int vInd = 0;
  int p1Ind = 0;
  int p2Ind = 0;

  V = 0;
  G = 0;

  switch ( Mesh.DIM )
  {
    case 3 :
    {
      int vDOF = Mesh.DOF[1] + Mesh.DOF[2] + Mesh.DOF[3];
      // Geometric properties for volume averaging
      xmin = Mesh.PCellCenters[ idx2( 0, 0, Mesh.CellCentersLDI ) ];
      xmax = xmin;
      ymin = Mesh.PCellCenters[ idx2( 0, 1, Mesh.CellCentersLDI ) ];
      ymax = ymin;
      zmin = Mesh.PCellCenters[ idx2( 0, 2, Mesh.CellCentersLDI ) ];
      zmax = zmin;

      for (int cl = 0; cl < Mesh.DOF[1]; cl++)
      {
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmin) {
          xmin = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmax) {
          xmax = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
      }
      double L = xmax - xmin;
      for (int cl = 0; cl < Mesh.DOF[2]; cl++)
      {
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymin) {
          ymin = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymax) {
          ymax = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
      }
      double W = ymax - ymin;
      for (int cl = 0; cl < Mesh.DOF[3]; cl++)
      {
        if (Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] < zmin) {
          zmin = Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] > zmax) {
          zmax = Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ];
        }
      }
      double H = zmax - zmin;

      // Adjust min and max to avoid potential recirculation near boundaries.
      // Then compute mid and midRange.
      xmin = xmin + r*L;
      xmax = xmax - r*L;
      zmin = zmin + r*H;
      zmax = zmax - r*H;
      ymin = ymin + r*W;
      ymax = ymax - r*W;
      ymid = 0.5 * (ymin + ymax);
      midRangey = 0.5 * (ymax + ymid) - 0.5 * (ymid + ymin);

      // Compute averages
      for (int cl = 0; cl < Mesh.DOF[2]; cl++)
      {
        pNode1 = Mesh.VCellPressureNeighbor[ idx2( cl, 0, Mesh.VelocityCellPressureNeighborLDI ) ];
        pNode2 = Mesh.VCellPressureNeighbor[ idx2( cl, 1, Mesh.VelocityCellPressureNeighborLDI ) ];
        if (!Mesh.ImmersedBoundary[ pNode1 ] && !Mesh.ImmersedBoundary[ pNode2 ])
        {
          if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmin)
          {
            if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmax) // Inside x limits
            {
              if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymin)
              {
                if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymax) // Inside y limits
                {
                  if (Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] > zmin)
                 {
                    if (Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] < zmax) // Inside z limits
                    {
                      V = V + Solution[ cl ];
                      vInd++;
                      pressure = 0.5 * (Solution[ pNode1 + vDOF ] + Solution[ pNode2 + vDOF ] );
                      if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < ymid)
                      {
                        P1 = P1 + pressure;
                        p1Ind++;
                      }
                      else if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > ymid)
                      {
                        P2 = P2 + pressure;
                        p2Ind++;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      V = V / vInd;
      P1 = P1 / p1Ind;
      P2 = P2 / p2Ind;
      G = (P1 - P2) / midRangey;

      K = V / G;

      // Write out K
      std::ofstream KConstantY;
      KConstantY.open("KConstantY.dat");
      KConstantY << K;
      KConstantY.close();
      break;
    }
    case 2 :
    {
      int vDOF = Mesh.DOF[1] + Mesh.DOF[2];
      // Geometric properties for volume averaging
      xmin = Mesh.PCellCenters[ idx2( 0, 0, Mesh.CellCentersLDI ) ];
      xmax = xmin;
      ymin = Mesh.PCellCenters[ idx2( 0, 1, Mesh.CellCentersLDI ) ];
      ymax = ymin;

      for (int cl = 0; cl < Mesh.DOF[1]; cl++)
      {
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmin) {
          xmin = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmax) {
          xmax = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
        }
      }
      double L = xmax - xmin;
      for (int cl = 0; cl < Mesh.DOF[2]; cl++)
      {
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymin) {
          ymin = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
        if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymax) {
          ymax = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
        }
      }
      double W = ymax - ymin;

      // Adjust min and max to avoid potential recirculation near boundaries.
      // Then compute mid and midRange.
      xmin = xmin + r*L;
      xmax = xmax - r*L;
      ymin = ymin + r*W;
      ymax = ymax - r*W;
      ymid = 0.5 * (ymin + ymax);
      midRangey = 0.5 * (ymax + ymid) - 0.5 * (ymid + ymin);

      // Compute averages
      for (int cl = 0; cl < Mesh.DOF[2]; cl++)
      {
        pNode1 = Mesh.VCellPressureNeighbor[ idx2( cl, 0, Mesh.VelocityCellPressureNeighborLDI ) ];
        pNode2 = Mesh.VCellPressureNeighbor[ idx2( cl, 1, Mesh.VelocityCellPressureNeighborLDI ) ];
        if (!Mesh.ImmersedBoundary[ pNode1 ] && !Mesh.ImmersedBoundary[ pNode2 ])
        {
          if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmin)
          {
            if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmax) // Inside x limits
            {
              if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymin)
              {
                if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymax) // Inside y limits
                {
                  V = V + Solution[ cl ];
                  vInd++;
                  pressure = 0.5 * (Solution[ pNode1 + vDOF ] + Solution[ pNode2 + vDOF ] );
                  if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < ymid)
                  {
                    P1 = P1 + pressure;
                    p1Ind++;
                  }
                  else if (Mesh.VCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > ymid)
                  {
                    P2 = P2 + pressure;
                    p2Ind++;
                  }
                }
              }
            }
          }
        }
      }
      V = V / vInd;
      P1 = P1 / p1Ind;
      P2 = P2 / p2Ind;
      G = (P1 - P2) / midRangey;

      K = V / G;

      // Write out K
      std::ofstream KConstantY;
      KConstantY.open("KConstantY.dat");
      KConstantY << K;
      KConstantY.close();
      break;
    }
  }
}

void
computeAveragesZ ( const FluidMesh& Mesh, \
                    const paralution::LocalVector<double>& Solution, \
                    double& V, double& G )
{
  double xmin, xmax, midRangez, ymin, ymax, zmid, zmin, zmax, K, pNode1, pNode2;
  double P1 = 0;
  double P2 = 0;
  double r = 0.09;
  double pressure;
  int vInd = 0;
  int p1Ind = 0;
  int p2Ind = 0;

  V = 0;
  G = 0;

  int vDOF = Mesh.DOF[1] + Mesh.DOF[2] + Mesh.DOF[3];
  // Geometric properties for volume averaging
  xmin = Mesh.PCellCenters[ idx2( 0, 0, Mesh.CellCentersLDI ) ];
  xmax = xmin;
  ymin = Mesh.PCellCenters[ idx2( 0, 1, Mesh.CellCentersLDI ) ];
  ymax = ymin;
  zmin = Mesh.PCellCenters[ idx2( 0, 2, Mesh.CellCentersLDI ) ];
  zmax = zmin;

  for (int cl = 0; cl < Mesh.DOF[1]; cl++)
  {
    if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmin) {
      xmin = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
    }
    if (Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmax) {
      xmax = Mesh.UCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ];
    }
  }
  double L = xmax - xmin;
  for (int cl = 0; cl < Mesh.DOF[2]; cl++)
  {
    if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymin) {
      ymin = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
    }
    if (Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymax) {
      ymax = Mesh.VCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ];
    }
  }
  double W = ymax - ymin;
  for (int cl = 0; cl < Mesh.DOF[3]; cl++)
  {
    if (Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] < zmin) {
      zmin = Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ];
    }
    if (Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] > zmax) {
      zmax = Mesh.VCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ];
    }
  }
  double H = zmax - zmin;

  // Adjust min and max to avoid potential recirculation near boundaries.
  // Then compute mid and midRange.
  xmin = xmin + r*L;
  xmax = xmax - r*L;
  zmin = zmin + r*H;
  zmax = zmax - r*H;
  ymin = ymin + r*W;
  ymax = ymax - r*W;
  zmid = 0.5 * (zmin + zmax);
  midRangez = 0.5 * (zmax + zmid) - 0.5 * (zmid + zmin);

  // Compute averages
  for (int cl = 0; cl < Mesh.DOF[3]; cl++)
  {
    pNode1 = Mesh.WCellPressureNeighbor[ idx2( cl, 0, Mesh.VelocityCellPressureNeighborLDI ) ];
    pNode2 = Mesh.WCellPressureNeighbor[ idx2( cl, 1, Mesh.VelocityCellPressureNeighborLDI ) ];
    if (!Mesh.ImmersedBoundary[ pNode1 ] && !Mesh.ImmersedBoundary[ pNode2 ])
    {
      if (Mesh.WCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > xmin)
      {
        if (Mesh.WCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < xmax) // Inside x limits
        {
          if (Mesh.WCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] > ymin)
          {
            if (Mesh.WCellCenters[ idx2( cl, 1, Mesh.CellCentersLDI ) ] < ymax) // Inside y limits
            {
              if (Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] > zmin)
              {
                if (Mesh.WCellCenters[ idx2( cl, 2, Mesh.CellCentersLDI ) ] < zmax) // Inside z limits
                {
                  V = V + Solution[ cl ];
                  vInd++;
                  pressure = 0.5 * (Solution[ pNode1 + vDOF ] + Solution[ pNode2 + vDOF ] );
                  if (Mesh.WCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] < zmid)
                  {
                    P1 = P1 + pressure;
                    p1Ind++;
                  }
                  else if (Mesh.WCellCenters[ idx2( cl, 0, Mesh.CellCentersLDI ) ] > zmid)
                  {
                    P2 = P2 + pressure;
                    p2Ind++;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  V = V / vInd;
  P1 = P1 / p1Ind;
  P2 = P2 / p2Ind;
  G = (P1 - P2) / midRangez;

  K = V / G;

  // Write out K
  std::ofstream KConstantZ;
  KConstantZ.open("KConstantZ.dat");
  KConstantZ << K;
  KConstantZ.close();
}

void
computeKConstantDrive ( const FluidMesh & Mesh, \
                        const paralution::LocalVector<double>& Solution,
                        int direction )
{
  double V, G;
  switch ( direction )
  {
    case 0 :
      computeAveragesX ( Mesh, Solution, V, G );
      break;
    case 1 :
      computeAveragesY ( Mesh, Solution, V, G );
      break;
    case 2 :
      computeAveragesZ ( Mesh, Solution, V, G );
      break;
  }
}


void
writeSolutionL ( const FluidMesh& Mesh, const paralution::LocalVector<double>& sol, \
                 std::string& outName )
{
  double uval, vval, wval;
  std::ofstream flowrun;
  flowrun.open ( outName.c_str() );
  flowrun << "Title = Stokes Solution\n";
  switch ( Mesh.DIM )
  {
    case 2 :
    {
      flowrun << "VARIABLES = X, Y, P, U, V, IB\n";
      flowrun << "ZONE I=" << Mesh.NX << ", J=" << Mesh.NY << ", K=1, F=Point\n";
      for (int row = 0; row < Mesh.DOF[0]; row++)
      {
        uval = 0.5 * (sol[ Mesh.PressureCellUNeighbor[ idx2( row, 0, 2 ) ] ] \
                    + sol[ Mesh.PressureCellUNeighbor[ idx2( row, 1, 2 ) ] ] );
        vval = 0.5 * (sol[ Mesh.PressureCellVNeighbor[ idx2( row, 0, 2 ) ] + Mesh.DOF[1] ] \
                    + sol[ Mesh.PressureCellVNeighbor[ idx2( row, 1, 2 ) ] + Mesh.DOF[1] ] );
        flowrun << Mesh.PCellCenters[ idx2( row, 0, Mesh.CellCentersLDI ) ] << "\t";
        flowrun << Mesh.PCellCenters[ idx2( row, 1, Mesh.CellCentersLDI ) ] << "\t";
        flowrun << sol[ row + Mesh.DOF[1] + Mesh.DOF[2] ] << "\t";
        flowrun << uval << "\t";
        flowrun << vval << "\t";
        flowrun << Mesh.ImmersedBoundary[ row ] << "\n";
      }
      break;
    }
    case 3 :
    {
      flowrun << "VARIABLES = X, Y, Z, P, U, V, W, IB\n";
      flowrun << "ZONE I=" << Mesh.NX << ", J=" << Mesh.NY << ", K=" << Mesh.NZ << ", F=Point\n";
      for (int row = 0; row < Mesh.DOF[0]; row++)
      {
        uval = 0.5 * (sol[ Mesh.PressureCellUNeighbor[ idx2( row, 0, 2 ) ] ] \
                    + sol[ Mesh.PressureCellUNeighbor[ idx2( row, 1, 2 ) ] ] );
        vval = 0.5 * (sol[ Mesh.PressureCellVNeighbor[ idx2( row, 0, 2 ) ] + Mesh.DOF[1] ] \
                    + sol[ Mesh.PressureCellVNeighbor[ idx2( row, 1, 2 ) ] + Mesh.DOF[1] ] );
        wval = 0.5 * (sol[ Mesh.PressureCellWNeighbor[ idx2( row, 0, 2 ) ] \
                           + Mesh.DOF[1] + Mesh.DOF[2] ] \
                    + sol[ Mesh.PressureCellWNeighbor[ idx2( row, 1, 2 ) ] \
                           + Mesh.DOF[1] + Mesh.DOF[2] ] );
        flowrun << Mesh.PCellCenters[ idx2( row, 0, Mesh.CellCentersLDI ) ] << "\t";
        flowrun << Mesh.PCellCenters[ idx2( row, 1, Mesh.CellCentersLDI ) ] << "\t";
        flowrun << Mesh.PCellCenters[ idx2( row, 2, Mesh.CellCentersLDI ) ] << "\t";
        flowrun << sol[ row + Mesh.DOF[1] + Mesh.DOF[2] + Mesh.DOF[3] ] << "\t";
        flowrun << uval << "\t";
        flowrun << vval << "\t";
        flowrun << wval << "\t";
        flowrun << Mesh.ImmersedBoundary[ row ] << "\n";
      }
      break;
    }
  }
  flowrun.close();
}