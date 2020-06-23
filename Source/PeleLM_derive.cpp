#include "PeleLM.H"
#include "PeleLM_derive.H"
#include <EOS.H>

using namespace amrex;


//
// Multiply by Rho
//

void pelelm_dermprho (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                  const FArrayBox& datfab, const Geometry& /*geomdata*/,
                  Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= 2);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        der(i,j,k) = in_dat(i,j,k,1)*in_dat(i,j,k,0);
    });
}

//
// Divide by Rho
//

void pelelm_derdvrho (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                  const FArrayBox& datfab, const Geometry& /*geomdata*/,
                  Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= 2);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        der(i,j,k) = in_dat(i,j,k,1)/in_dat(i,j,k,0);
    });
}


//
// rhoY_n, identity ---------- FIXME?
//

void pelelm_derRhoY (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                  const FArrayBox& datfab, const Geometry& /*geomdata*/,
                  Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES);
    AMREX_ASSERT(ncomp == NUM_SPECIES);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx, NUM_SPECIES,
    [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
    {
        der(i,j,k,n) = in_dat(i,j,k,n);
    });
}

//
// Extract species mass fractions Y_n
//

void pelelm_dermassfrac (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                  const FArrayBox& datfab, const Geometry& /*geomdata*/,
                  Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES+1);
    AMREX_ASSERT(ncomp == NUM_SPECIES);
    auto const in_dat = datfab.array();
    auto       der = derfab.array(dcomp);
    amrex::ParallelFor(bx, NUM_SPECIES,
    [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
    {
        amrex::Real rhoinv = 1.0 / in_dat(i,j,k,0);
        der(i,j,k,n) = in_dat(i,j,k,n+1) * rhoinv;
    });
}

//
// Extract species mole fractions X_n
//

void pelelm_dermolefrac (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                  const FArrayBox& datfab, const Geometry& /*geomdata*/,
                  Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES+1);
    AMREX_ASSERT(ncomp == NUM_SPECIES);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx, 
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        amrex::Real Yt[NUM_SPECIES];
        amrex::Real Xt[NUM_SPECIES];
        amrex::Real rhoinv = 1.0 / in_dat(i,j,k,0);
        for (int n = 0; n < NUM_SPECIES; n++) {
          Yt[n] = in_dat(i,j,k,n+1) * rhoinv;
        } 
        EOS::Y2X(Yt,Xt);
        for (int n = 0; n < NUM_SPECIES; n++) {
          der(i,j,k,n) = Xt[n];
        }
    });

}

//
// Compute the mixture mean molecular weight
//

void pelelm_dermolweight (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                  const FArrayBox& datfab, const Geometry& /*geomdata*/,
                  Real /*time*/, const int* /*bcrec*/, int /*level*/)

{   
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES+1);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx, 
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {   
        amrex::Real Yt[NUM_SPECIES];
        amrex::Real Wbar;
        amrex::Real rhoinv = 1.0 / in_dat(i,j,k,0);
        for (int n = 0; n < NUM_SPECIES; n++) {
          Yt[n] = in_dat(i,j,k,n+1) * rhoinv;
        } 
        EOS::Y2WBAR(Yt,Wbar);
        der(i,j,k) = Wbar;
    });

}

//
// Compute the mixture mean heat capacity at cst pressure
//

void pelelm_dercpmix (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                      const FArrayBox& datfab, const Geometry& /*geomdata*/,
                      Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES+2);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        amrex::Real Yt[NUM_SPECIES];
        amrex::Real Temp;
        amrex::Real Cpmix;
        amrex::Real rhoinv = 1.0 / in_dat(i,j,k,0);
        for (int n = 0; n < NUM_SPECIES; n++) {
          Yt[n] = in_dat(i,j,k,n+2) * rhoinv;
        }
        Temp = in_dat(i,j,k,1);
        EOS::TY2Cp(Temp,Yt,Cpmix);
        der(i,j,k) = Cpmix * 1.0e-4; // CGS -> MKS ;
    });

}

//
// Compute rho - Sum_n(rhoY_n)
//

void pelelm_drhomry (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                     const FArrayBox& datfab, const Geometry& /*geomdata*/,
                     Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES+1);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    // we put the density in the component 0 of der array
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        der(i,j,k) = in_dat(i,j,k,0);
    });
    
    // we substract to rho with every rhoY(n)
    amrex::ParallelFor(bx, NUM_SPECIES,
    [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
    {
        der(i,j,k) -= in_dat(i,j,k,n+1);
    });
}

//
// Compute sum of rhoY_dot
//

void pelelm_dsrhoydot (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                       const FArrayBox& datfab, const Geometry& /*geomdata*/,
                       Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(datfab.box().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    AMREX_ASSERT(in_dat.nComp()==NUM_SPECIES);

    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        der(i,j,k) = 0.;
    });

    amrex::ParallelFor(bx, NUM_SPECIES,
    [=] AMREX_GPU_DEVICE (int i, int j, int k, int n) noexcept
    {
        der(i,j,k) += in_dat(i,j,k,n);
    });
}


//
//  Compute cell-centered pressure as average of the 
//  surrounding nodal values 
//

void pelelm_deravgpres (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                        const FArrayBox& datfab, const Geometry& /*geomdata*/,
                        Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(Box(datfab.box()).enclosedCells().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= 1);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
#if (AMREX_SPACEDIM == 2 )
    Real factor = 0.25;
#elif (AMREX_SPACEDIM == 3 )
    Real factor = 0.125;
#endif

    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        der(i,j,k) =  factor * (  in_dat(i+1,j,k)     + in_dat(i,j,k)
                                + in_dat(i+1,j+1,k)   + in_dat(i,j+1,k)
#if (AMREX_SPACEDIM == 3 )
                                + in_dat(i+1,j,k+1)   + in_dat(i,j,k+1)
                                + in_dat(i+1,j+1,k+1) + in_dat(i,j+1,k+1)
#endif
                                );
    });
}


//
//   Compute cell centered gradient of the nodal pressure in direction x
//

void pelelm_dergrdpx (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                      const FArrayBox& datfab, const Geometry& geomdata,
                      Real /*time*/, const int* /*bcrec*/, int /*level*/)

{   
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(Box(datfab.box()).enclosedCells().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= 1);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    const auto dxinv = geomdata.InvCellSizeArray();    
#if (AMREX_SPACEDIM == 2 )
    Real factor = 0.5*dxinv[0];
#elif (AMREX_SPACEDIM == 3 )
    Real factor = 0.25*dxinv[0];
#endif
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {   
#if (AMREX_SPACEDIM == 2 )
        der(i,j,k) = factor*(in_dat(i+1,j,k)-in_dat(i,j,k)+in_dat(i+1,j+1,k)-in_dat(i,j+1,k));
#elif (AMREX_SPACEDIM == 3 )
        der(i,j,k) = factor*( in_dat(i+1,j,k)-in_dat(i,j,k)+in_dat(i+1,j+1,k)-in_dat(i,j+1,k) +
                              in_dat(i+1,j,k+1)-in_dat(i,j,k+1)+in_dat(i+1,j+1,k+1)-in_dat(i,j+1,k+1));
#endif 
    });
}

//
//   Compute cell centered gradient of the nodal pressure in direction y
//

void pelelm_dergrdpy (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                      const FArrayBox& datfab, const Geometry& geomdata,
                      Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(Box(datfab.box()).enclosedCells().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= 1);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    const auto dxinv = geomdata.InvCellSizeArray();
#if (AMREX_SPACEDIM == 2 )
    Real factor = 0.5*dxinv[1];
#elif (AMREX_SPACEDIM == 3 )
    Real factor = 0.25*dxinv[1];
#endif
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
#if (AMREX_SPACEDIM == 2 )
        der(i,j,k) = factor*(in_dat(i,j+1,k)-in_dat(i,j,k)+in_dat(i+1,j+1,k)-in_dat(i+1,j,k));
#elif (AMREX_SPACEDIM == 3 )
        der(i,j,k) = factor*( in_dat(i,j+1,k)-in_dat(i,j,k)+in_dat(i+1,j+1,k)-in_dat(i+1,j,k) +
                              in_dat(i,j+1,k+1)-in_dat(i,j,k+1)+in_dat(i+1,j+1,k+1)-in_dat(i+1,j,k+1));
#endif
    });
}


//
//   Compute cell centered gradient of the nodal pressure in direction z
//

void pelelm_dergrdpz (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                      const FArrayBox& datfab, const Geometry& geomdata,
                      Real /*time*/, const int* /*bcrec*/, int /*level*/)

{
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(Box(datfab.box()).enclosedCells().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= 1);
    AMREX_ASSERT(ncomp == 1);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    const auto dxinv = geomdata.InvCellSizeArray();
    Real factor = 0.25*dxinv[2];
    amrex::ParallelFor(bx,
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {
        der(i,j,k) = factor*( in_dat(i,  j,k+1)-in_dat(i,  j,k)+in_dat(i,  j+1,k+1)-in_dat(i,  j+1,k) +
                              in_dat(i+1,j,k+1)-in_dat(i+1,j,k)+in_dat(i+1,j+1,k+1)-in_dat(i+1,j+1,k));
    });
}



//
// Compute species concentrations C_n
//

void pelelm_derconcentration (const Box& bx, FArrayBox& derfab, int dcomp, int ncomp,
                              const FArrayBox& datfab, const Geometry& /*geomdata*/,
                              Real /*time*/, const int* /*bcrec*/, int /*level*/)

{   
    AMREX_ASSERT(derfab.box().contains(bx));
    AMREX_ASSERT(Box(datfab.box()).enclosedCells().contains(bx));
    AMREX_ASSERT(derfab.nComp() >= dcomp + ncomp);
    AMREX_ASSERT(datfab.nComp() >= NUM_SPECIES+2);
    AMREX_ASSERT(ncomp == NUM_SPECIES);
    auto const in_dat = datfab.array();
    auto          der = derfab.array(dcomp);
    amrex::ParallelFor(bx, 
    [=] AMREX_GPU_DEVICE (int i, int j, int k) noexcept
    {   
        amrex::Real Yt[NUM_SPECIES];
        amrex::Real Ct[NUM_SPECIES];
        amrex::Real Temp;
        amrex::Real Rho;
        amrex::Real rhoinv = 1.0 / in_dat(i,j,k,0);
        for (int n = 0; n < NUM_SPECIES; n++) {
          Yt[n] = in_dat(i,j,k,n+2) * rhoinv;
        }
        Temp = in_dat(i,j,k,1);
        Rho = in_dat(i,j,k,0) * 1.0e-3; // ! kg/m^3 -> g/cm^3
        EOS::RTY2C(Rho,Temp,Yt,Ct);
        for (int n = 0; n < NUM_SPECIES; n++) {
          der(i,j,k,n) = Ct[n] * 1.0e6; // cm^(-3) -> m^(-3)
        }
    });
}