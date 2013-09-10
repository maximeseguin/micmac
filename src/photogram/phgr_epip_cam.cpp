/*Header-MicMac-eLiSe-25/06/2007

    MicMac : Multi Image Correspondances par Methodes Automatiques de Correlation
    eLiSe  : ELements of an Image Software Environnement

    www.micmac.ign.fr

   
    Copyright : Institut Geographique National
    Author : Marc Pierrot Deseilligny
    Contributors : Gregoire Maillet, Didier Boldo.

[1] M. Pierrot-Deseilligny, N. Paparoditis.
    "A multiresolution and optimization-based image matching approach:
    An application to surface reconstruction from SPOT5-HRS stereo imagery."
    In IAPRS vol XXXVI-1/W41 in ISPRS Workshop On Topographic Mapping From Space
    (With Special Emphasis on Small Satellites), Ankara, Turquie, 02-2006.

[2] M. Pierrot-Deseilligny, "MicMac, un lociel de mise en correspondance
    d'images, adapte au contexte geograhique" to appears in 
    Bulletin d'information de l'Institut Geographique National, 2007.

Francais :

   MicMac est un logiciel de mise en correspondance d'image adapte 
   au contexte de recherche en information geographique. Il s'appuie sur
   la bibliotheque de manipulation d'image eLiSe. Il est distibue sous la
   licences Cecill-B.  Voir en bas de fichier et  http://www.cecill.info.


English :

    MicMac is an open source software specialized in image matching
    for research in geographic information. MicMac is built on the
    eLiSe image library. MicMac is governed by the  "Cecill-B licence".
    See below and http://www.cecill.info.

Header-MicMac-eLiSe-25/06/2007*/

#define NoTemplateOperatorVirgule
#define NoSimpleTemplateOperatorVirgule

#include "StdAfx.h"

class cEpipOrientCam : public  ElDistortion22_Gen
{
	friend class PourFairPlaisirAGCC;
	public :
           cEpipOrientCam
	   (
	         bool ToDel,
		 REAL aZoom,
	         CamStenope & aCam,
                 const ElMatrix<REAL> & aRot
           );

           virtual bool OwnInverse(Pt2dr &) const ;    //  Epi vers Phot
           virtual Pt2dr Direct(Pt2dr) const  ;    // Photo vers Epi
           cEpipOrientCam MapingChScale(REAL aChSacle) const;


	private :
	   virtual ~cEpipOrientCam();
           virtual ElDistortion22_Gen  * D22G_ChScale(REAL aS) const; //
           void  Diff(ElMatrix<REAL> &,Pt2dr) const ;  //  Erreur Fatale

	   bool               m2Del;
	   REAL               mZoom;
	   CamStenope *       mCamInit;
	   CamStenopeIdeale   mCamEpip;

};

// Orientation monde->Cam, pour l'entree et la sortie
ElMatrix<REAL>  OrientationEpipolaire(ElRotation3D R1,ElRotation3D R2);

/********************************************************/
/*                                                      */
/*       cEpipOrientCam                                 */
/*                                                      */
/********************************************************/

Pt2dr cEpipOrientCam::Direct(Pt2dr aP) const  
{
   Pt3dr R1,R2;
   mCamInit->F2toRayonR3(aP/mZoom,R1,R2);

   return mCamEpip.R3toF2(R2)*mZoom;
}

bool cEpipOrientCam::OwnInverse(Pt2dr & aP) const     //  Epi vers Phot
{
   Pt3dr R1,R2;
   mCamEpip.F2toRayonR3(aP/mZoom,R1,R2);
   aP  =  mCamInit->R3toF2(R2)*mZoom;
   return true;
}

cEpipOrientCam cEpipOrientCam::MapingChScale(REAL aChSacle) const
{
	return cEpipOrientCam
		(
		    false,
                    mZoom * aChSacle,
		    *mCamInit,
		    mCamEpip.Orient().Mat()
		);
}


cEpipOrientCam::~cEpipOrientCam()
{
   if (m2Del)
      delete mCamInit;
}

ElDistortion22_Gen  * cEpipOrientCam::D22G_ChScale(REAL aS) const
{
    cEpipOrientCam  aNewS = MapingChScale(aS);
    return new cEpipOrientCam(aNewS);
}

void  cEpipOrientCam::Diff(ElMatrix<REAL> & ,Pt2dr) const 
{
      ELISE_ASSERT(false,"No cEpipOrientCam::Diff");
}

cEpipOrientCam::cEpipOrientCam
(
	         bool ToDel,
		 REAL aZoom,
	         CamStenope & aCam,
                 const ElMatrix<REAL> & aMat
) :
  m2Del    (ToDel),
  mZoom    (aZoom),
  mCamInit (&aCam),
  mCamEpip (true,aCam.Focale(),aCam.PP(),aCam.ParamAF())
{
   ELISE_ASSERT(!aCam.UseAFocal(),"AFocal epip");
   ElRotation3D aRot(Pt3dr(0,0,0),aMat,true);
   ElMatrix<REAL> MR = aRot.Mat();

   Pt3dr Tr = -(MR*aCam.VraiOpticalCenter());

   mCamEpip.SetOrientation(ElRotation3D(Tr,MR,true));
}

/********************************************************/
/*                                                      */
/*                ::                                    */
/*                                                      */
/********************************************************/

ElMatrix<REAL>  OrientationEpipolaire(ElRotation3D R1,ElRotation3D R2,int aSign,double & aD)
{
     Pt3dr COpt1 = R1.ImRecAff(Pt3dr(0,0,0));
     Pt3dr COpt2 = R2.ImRecAff(Pt3dr(0,0,0));

     Pt3dr Ox = vunit((COpt2 - COpt1)*aSign);

     Pt3dr OZ1 = R1.IRecVect(Pt3dr(0,0,1));
     Pt3dr OZ2 = R2.IRecVect(Pt3dr(0,0,1));
     Pt3dr Oz = vunit(OZ2+OZ1) ;

     Pt3dr Oy = vunit(Oz ^ Ox);
     Oz = vunit(Ox ^ Oy);


     ElMatrix<REAL> aRes(3,3);
     SetCol(aRes,0,Ox);
     SetCol(aRes,1,Oy);
     SetCol(aRes,2,Oz);

     aRes =  aRes.transpose();

     aD  = aRes.L2(R1.Mat()) + aRes.L2(R2.Mat());
     return aRes;
}


ElMatrix<REAL>  OrientationEpipolaire(ElRotation3D R1,ElRotation3D R2)
{
    double aD1;
    ElMatrix<REAL> aR1 =  OrientationEpipolaire(R1,R2,1,aD1);

    double aD2;
    ElMatrix<REAL> aR2 =  OrientationEpipolaire(R1,R2,-1,aD2);

    // std::cout << "DDDddddD "<< aD1 << " " << aD2 << "\n";

    return (aD1<aD2)  ? aR1 : aR2 ;



/*
     Pt3dr COpt1 = R1.ImRecAff(Pt3dr(0,0,0));
     Pt3dr COpt2 = R2.ImRecAff(Pt3dr(0,0,0));

     Pt3dr Ox = vunit(COpt2 - COpt1);

     Pt3dr OX12 = vunit(R1.IRecVect(Pt3dr(1,0,0)) +R2.IRecVect(Pt3dr(1,0,0)));
     if (scal(Ox,OX12) <0)
         Ox =  -Ox;

     Pt3dr OZ1 = R1.IRecVect(Pt3dr(0,0,1));
     Pt3dr OZ2 = R2.IRecVect(Pt3dr(0,0,1));
     Pt3dr Oz = vunit(OZ2+OZ1) ;

     Oz = vunit(Oz - Ox * scal(Ox,Oz));

     Pt3dr Oy = Oz ^ Ox;

     ElMatrix<REAL> aRes(3,3);

     SetCol(aRes,0,Ox);
     SetCol(aRes,1,Oy);
     SetCol(aRes,2,Oz);

     return aRes.transpose();
*/
}

/********************************************************/
/*                                                      */
/*                CpleEpipolaireCoord                   */
/*                                                      */
/********************************************************/


CpleEpipolaireCoord * CpleEpipolaireCoord::CamEpipolaire
                      (
                          CamStenope  & aCam1, Pt2dr aP1,
                          CamStenope  & aCam2, Pt2dr aP2,
                          REAL aZoom
                      )
{
   aP1 = aP1 * aZoom;
   aP2 = aP2 * aZoom;
   ElMatrix<REAL> aMat =  OrientationEpipolaire(aCam1.Orient(),aCam2.Orient());

   cEpipOrientCam * OR1 = new cEpipOrientCam(false,aZoom,aCam1,aMat);
   cEpipOrientCam * OR2 = new cEpipOrientCam(false,aZoom,aCam2,aMat);

    cMappingEpipCoord * aMap1 = new cMappingEpipCoord(OR1,true);
    cMappingEpipCoord * aMap2 = new cMappingEpipCoord(OR2,true);


    Pt2dr aQ1 = aMap1->Direct(aP1);
    Pt2dr aQ2 = aMap2->Direct(aP2);

    aMap1->AddTrFinale(Pt2dr(aQ2.x-aQ1.x,0));
    return new CpleEpipolaireCoord(aMap1,aMap2);
}

/********************************************************/
/*                                                      */
/*                   ::                                 */
/*                                                      */
/********************************************************/



Pt2dr GlobTransfoEpip ( const Pt2dr & aPIm, const CamStenope & aCamIn, const CamStenope & aCamOut) 
{

    Pt3dr aC = aCamIn.VraiOpticalCenter();
    Pt3dr aRay = aCamIn.F2toDirRayonR3(aPIm);
    return  aCamOut.R3toF2(aC+aRay);
}

Box2dr  GlobBoxCam(Box2dr aBoxIn,const CamStenope & aCamIn,const CamStenope & aCamOut) 
{
    std::vector<Pt2dr> aVPtsIn;
    int aNbPts =1;
    aBoxIn.PtsDisc(aVPtsIn,aNbPts);

    Pt2dr aPInfOut(1e20,1e20);
    Pt2dr aPSupOut(-1e20,-1e20);

    for (int aK=0 ; aK<int(aVPtsIn.size()) ; aK++)
    {
        Pt2dr aP = GlobTransfoEpip(aVPtsIn[aK],aCamIn,aCamOut);
        aPInfOut.SetInf(aP);
        aPSupOut.SetSup(aP);
    }

    return Box2dr(aPInfOut,aPSupOut);
}
/********************************************************/
/*                                                      */
/*                cCpleEpip                             */
/*                                                      */
/********************************************************/


CamStenopeIdeale  cCpleEpip::CamOut(const CamStenope & aCamIn,Pt2dr aPP,Pt2di aSz)
{
    ElRotation3D  aRC2M(aCamIn.VraiOpticalCenter(),mMatC2M,true);
    CamStenopeIdeale  aCamOut(aCamIn.DistIsC2M(),mFoc,aPP,aCamIn.ParamAF());
    aCamOut.SetOrientation(aRC2M.inv());
    aCamOut.SetSz(aSz);

    if (aCamIn.AltisSolIsDef())
       aCamOut.SetAltiSol(aCamIn.GetAltiSol());
    if (aCamIn.ProfIsDef())
       aCamOut.SetProfondeur(aCamIn.GetProfondeur());

    return aCamOut;
}

Box2dr  cCpleEpip::BoxCam(const CamStenope & aCamIn,const CamStenope & aCamOut,bool Show) const
{
    return GlobBoxCam(Box2dr(Pt2dr(0,0),Pt2dr(aCamIn.Sz())),aCamIn,aCamOut);
/*
    Box2dr aBoxIn (Pt2dr(0,0),Pt2dr(aCamIn.Sz()));
    std::vector<Pt2dr> aVPtsIn;
    int aNbPts =1;
    if (Show && (aNbPts!=10)) std::cout << "Xxxxxxxxxxx cCpleEpip::BoxCam \n";
    aBoxIn.PtsDisc(aVPtsIn,aNbPts);

    Pt2dr aPInfOut(1e20,1e20);
    Pt2dr aPSupOut(-1e20,-1e20);

    for (int aK=0 ; aK<int(aVPtsIn.size()) ; aK++)
    {
        Pt2dr aP = TransfoEpip(aVPtsIn[aK],aCamIn,aCamOut);
        if (Show)
           std::cout << "BoxCam " << aP << aVPtsIn[aK] << TransfoEpip(aP,aCamOut,aCamIn) << "\n";
        aPInfOut.SetInf(aP);
        aPSupOut.SetSup(aP);
    }

    return Box2dr(aPInfOut,aPSupOut);
*/
}





Pt2dr cCpleEpip::TransfoEpip ( const Pt2dr & aPIm, const CamStenope & aCamIn, const CamStenope & aCamOut) const
{
   return GlobTransfoEpip(aPIm,aCamIn,aCamOut);
}



const bool & cCpleEpip::Ok() const
{
   return mOk;
}

void cCpleEpip::AssertOk() const
{
    ELISE_ASSERT(mOk,"CpleEpip::AssertOk Not OK ");
}

//Box2di BoxEpip


cCpleEpip::cCpleEpip
(
   const std::string & aDir,
   double aScale,
   const CamStenope & aC1,  const std::string & aName1,
   const CamStenope & aC2,  const std::string & aName2,
   const std::string & aPrefLeft,
   const std::string & aPrefRight
)  :
   mScale    (aScale),
   mDir      (aDir),
   mICNM     (cInterfChantierNameManipulateur::BasicAlloc(aDir)),
   mCInit1   (aC1),
   mName1    (aName1),
   mCInit2   (aC2),
   mName2    (aName2),
   mNamePair (   (aName1<aName2) ? 
                 (StdPrefixGen(aName1)+ "_" +StdPrefixGen(aName2)) : 
                 (StdPrefixGen(aName2)+ "_" +StdPrefixGen(aName1))
             ),
   mPrefLeft (aPrefLeft),
   mPrefRight (aPrefRight),
   mSzIn     (Sup(mCInit1.Sz(),mCInit2.Sz())),
   mFoc      (sqrt(aC1.Focale()*aC2.Focale())/mScale),
   mMatM2C   (OrientationEpipolaire(mCInit1.Orient(),mCInit2.Orient())),
   mMatC2M   (mMatM2C.transpose()),
   mCamOut1  (CamOut(mCInit1,Pt2dr(0,0),mSzIn)),
   mCamOut2  (CamOut(mCInit2,Pt2dr(0,0),mSzIn)),
   mOk       (false)
{
   Box2dr aB1 = BoxCam(mCInit1,mCamOut1,false);
   Box2dr aB2 = BoxCam(mCInit2,mCamOut2,false);

   double yMin = ElMax(aB1._p0.y,aB2._p0.y);
   double yMax = ElMin(aB1._p1.y,aB2._p1.y);
   int aSzY =  round_ni(yMax-yMin);
   if (aSzY <=0)
      return;


   mCamOut1  = CamOut(mCInit1,-Pt2dr(aB1._p0.x,yMin),Pt2di(aB1.sz().x,aSzY));
   mCamOut2  = CamOut(mCInit2,-Pt2dr(aB2._p0.x,yMin),Pt2di(aB2.sz().x,aSzY));


   if (1)
   {
      Pt3dr aP1 =  aC1.ImEtProf2Terrain(Pt2dr(aC1.Sz()/2),aC1.GetProfondeur());
      Pt3dr aP2 =  aC2.ImEtProf2Terrain(Pt2dr(aC2.Sz()/2),aC2.GetProfondeur());
      Pt3dr aP = (aP1+aP2) / 2.0;
 
      Pt2dr aPI1 = mCamOut1.R3toF2(aP);
      Pt2dr aPI2 = mCamOut2.R3toF2(aP);
      double aDX = aPI2.x - aPI1.x;

      double aDX1 = (aDX > 0 ) ? 0 : (-aDX);
      double aDX2 = (aDX > 0 ) ? aDX : 0 ;

      int aSzX1 = aB1.sz().x - ElAbs(aDX);
      int aSzX2 = aB2.sz().x - ElAbs(aDX);

      if ((aSzX1<=0) || (aSzX2 <=0)) return;

      mCamOut1  = CamOut(mCInit1,-Pt2dr(aB1._p0.x+aDX1,yMin),Pt2di(aSzX1,aSzY));
      mCamOut2  = CamOut(mCInit2,-Pt2dr(aB2._p0.x+aDX2,yMin),Pt2di(aSzX2,aSzY));
   }

   Pt3dr aDirI =  mMatC2M * Pt3dr(1,0,0);
   Pt3dr aDirC = vunit(mCamOut2.VraiOpticalCenter() - mCamOut1.VraiOpticalCenter());

   mFirstIsLeft = (scal(aDirI,aDirC) > 0) ;

   mOk = true;
}


template <class Type,class TypeBase> class cReechantEpi
{
     public :
            cReechantEpi
            (
                 const CamStenope & aCamIn,
                 Tiff_Im aTIn,
                 const CamStenope & aCamOut,
                 Tiff_Im aTOut,
                 Box2di  aBoxOut
            );
            typedef TIm2D<Type,TypeBase>  tTIm;
     private  :
};

template <class Type,class TypeBase> 
         cReechantEpi<Type,TypeBase>::cReechantEpi
         ( 
             const CamStenope & aCamIn,
             Tiff_Im aTIn,
             const CamStenope & aCamOut,
             Tiff_Im aTOut,
             Box2di  aBoxOut
         )
{
   INT aRab = 10;
   Pt2di aPRab(aRab,aRab);
   Box2di aBoxIn = R2I(GlobBoxCam(I2R(aBoxOut),aCamOut,aCamIn));
   aBoxIn._p0 = Sup(Pt2di(0,0),aBoxIn._p0-aPRab);
   aBoxIn._p1 = Inf(aTIn.sz(),aBoxIn._p1+aPRab);
   Pt2di aSzIn = aBoxIn.sz();
   Pt2di aSzOut = aBoxOut.sz();

   int aNbChan = aTIn.nb_chan();



   std::vector<tTIm> aVIn;
   std::vector<Type**> aDataIn;
   std::vector<tTIm> aVOut;
   Output aOutInit = Output::onul(1);
   Fonc_Num  aFOut = 0;
   for (int aKC=0 ; aKC<aNbChan ; aKC++)
   {
      aVIn.push_back(tTIm(aSzIn));
      Output anOutK = aVIn.back()._the_im.out();
      aOutInit = (aKC==0) ? anOutK : Virgule(aOutInit,anOutK);
      aDataIn.push_back(aVIn.back()._the_im.data());

      aVOut.push_back(tTIm(aSzOut));
      Fonc_Num aFK = aVOut.back()._the_im.in();
      aFOut = (aKC==0) ? aFK : Virgule(aFOut,aFK);
   }
   ELISE_COPY(rectangle(Pt2di(0,0),aSzIn),trans(aTIn.in(),aBoxIn._p0),aOutInit);
 


   int aPas = 4;
   int aSzXR = 1+(aSzOut.x+ aPas-1) / aPas;
   int aSzXY = 1+(aSzOut.y+ aPas-1) / aPas;
   TIm2D<REAL8,REAL8> aTImX(Pt2di(aSzXR+1,aSzXY+1));
   TIm2D<REAL8,REAL8> aTImY(Pt2di(aSzXR+1,aSzXY+1));
   Pt2di aPInd;
   for ( aPInd.x=0; aPInd.x<=aSzXR ; aPInd.x++)
   {
       for (aPInd.y=0; aPInd.y<=aSzXY ; aPInd.y++)
       {
            Pt2dr aPIm = GlobTransfoEpip(Pt2dr(aPInd*aPas+aBoxOut._p0),aCamOut,aCamIn) -Pt2dr(aBoxIn._p0);
            aTImX.oset(aPInd,aPIm.x);
            aTImY.oset(aPInd,aPIm.y);
       }
   }

    cInterpolateurIm2D<Type> * aKern = 0;
    if (0)
    {
       aKern = new cInterpolBilineaire<Type>;
    }
    else if (1)
    {
       aKern = new cTplCIKTabul<Type,TypeBase>(10,8,-0.5);
    }

    double aSzK = aKern->SzKernel();
    double aTxKer = aSzIn.x - aSzK;
    double aTyKer = aSzIn.y - aSzK;


    double UnSPas = 1.0/aPas;
    for (int anX=0; anX<aSzOut.x ; anX++)
    {
       Pt2dr aPR(anX/double(aPas),0);
       for (int anY=0; anY<aSzOut.y ; anY++)
       {

            Pt2dr aPIm(aTImX.getr(aPR),aTImY.getr(aPR));
            bool Ok =    (aPIm.x > aSzK)
                      && (aPIm.y > aSzK)
                      && (aPIm.x< aTxKer)
                      && (aPIm.y< aTyKer);

     

            for (int aKC=0 ; aKC<aNbChan ; aKC++)
            {
                double aVal =  Ok ?  aKern->GetVal(aDataIn[aKC],aPIm) : 0 ;
                aVOut[aKC].oset(Pt2di(anX,anY),aVal);
                
            }
            aPR.y += UnSPas;
       }
    }
    ELISE_COPY
    (
         rectangle(aBoxOut._p0,aBoxOut._p1),
         trans(aFOut,-aBoxOut._p0),
         aTOut.out()
    );
    delete aKern;

}

void ReechEpipGen
     ( 
             const CamStenope & aCamIn,
             Tiff_Im aTIn,
             const CamStenope & aCamOut,
             Tiff_Im aTOut,
             Box2di  aBoxOut
     )
{
   switch (aTOut.type_el())
   {
        case GenIm::u_int1 :
        {
             cReechantEpi<U_INT1,INT> aREE1(aCamIn,aTIn,aCamOut,aTOut,aBoxOut);
        }
        break;

        case GenIm::u_int2 :
        {
             cReechantEpi<U_INT2,INT> aREE2(aCamIn,aTIn,aCamOut,aTOut,aBoxOut);
        }
        break;

        default :
             ELISE_ASSERT(false,"Numerical type not handled in cReechantEpi");
   }
}


int CreateBlockEpip_main(int argc,char ** argv)
{
   MMD_InitArgcArgv(argc,argv);

   std::string aNameTifIn,aNameTifOut,aNameCamIn,aNameCamOut;
   Box2di aBoxOut;

   bool mSinCard=false;

   ElInitArgMain
   (
        argc,argv,
        LArgMain()  << EAMC(aNameTifIn,"Tiff In")
                    << EAMC(aNameTifOut,"Tiff Out")
                    << EAMC(aNameCamIn,"Camera In")
                    << EAMC(aNameCamOut,"Camera Out")
                    << EAMC(aBoxOut,"Box out"),
        LArgMain()  << EAM(mSinCard,"SinCard","Use sinus card for interp, def = false")
   );

    CamStenope * aCamOut = CamOrientGenFromFile(aNameCamOut,0);
    CamStenope * aCamIn = CamOrientGenFromFile(aNameCamIn,0);

    Tiff_Im aTIn(aNameTifIn.c_str());
    Tiff_Im aTOut(aNameTifOut.c_str());


    ReechEpipGen(*aCamIn,aTIn,*aCamOut,aTOut,aBoxOut);

    return 0;
}
/*
*/



void cCpleEpip::ImEpip(Tiff_Im aTIn,const std::string & aNameOriIn,bool Im1)
{
    bool ByP= true;
    bool ImLeft = mFirstIsLeft ? Im1 : (!Im1) ;
    std::string  aNameImOut = mDir + "Epi_" + std::string(Im1 ? "Im1_" : "Im2_") + (ImLeft ? mPrefLeft : mPrefRight  ) +   mNamePair + ".tif";

    AssertOk();
    const CamStenope & aCamIn =        Im1 ? mCInit1  : mCInit2;
    const CamStenopeIdeale & aCamOut = Im1 ? mCamOut1 : mCamOut2;
    Pt2di aSzOut = aCamOut.Sz();


    std::string aNameOriOut =  mICNM->Assoc1To1("NKS-Assoc-Im2Orient@-Epi",NameWithoutDir(aNameImOut),true);
    // std::cout << "ORI = " << aNameOri << " " << aNameImOut << " " << NameWithoutDir(aNameImOut) << "\n";
    // std::cout << "ORI = " << mDir << " " << mICNM->Assoc1To1("NKS-Assoc-Im2Orient@-Epi",NameWithoutDir(aNameImOut),true) << "\n";
    cOrientationConique anOC = aCamOut.StdExportCalibGlob();
    MakeFileXML(anOC,mDir+aNameOriOut);
    CamStenope * aCTEST = CamOrientGenFromFile(aNameOriOut,mICNM);
    std::cout << "TTTTT " << aCTEST->Focale() << "\n";


    GenIm::type_el aTypeNOut =  aTIn.type_el();
    Tiff_Im aTOut
            (
                  aNameImOut.c_str(),
                  aSzOut,
                  aTypeNOut,
                  Tiff_Im::No_Compr,
                  aTIn.phot_interp()
            );



    cDecoupageInterv2D  aDec = cDecoupageInterv2D::SimpleDec(aSzOut,2000,0);
    std::list<std::string> aLCom;

    for (int aK=0 ; aK<aDec.NbInterv() ; aK++)
    {
         Box2di aBoxOut = aDec.KthIntervOut(aK);

         if (ByP)
         {
              std::string aCom =    MM3dBinFile("TestLib")
                                  + std::string(" BlocEpip ")
                                  + " " + aTIn.name()
                                  + " " +  aNameImOut
                                  + " " + mDir + aNameOriIn
                                  + " " + mDir + aNameOriOut
                                  + " " + ToString(aBoxOut);
             aLCom.push_back(aCom);
         }
         else
         {
              ReechEpipGen (aCamIn,aTIn,aCamOut,aTOut,aBoxOut);
         }
    }
    cEl_GPAO::DoComInParal(aLCom,"MakeEpip");



#if (0)

    if (0)
    {
         Box2di aBoxOut(Pt2di(0,0),aSzOut);
         cReechantEpi<U_INT1,INT> aREE1(aCamIn,aTIn,aCamOut,aTOut,aBoxOut);
         cReechantEpi<U_INT2,INT> aREE2(aCamIn,aTIn,aCamOut,aTOut,aBoxOut);
         cReechantEpi<float,double> aREE3(aCamIn,aTIn,aCamOut,aTOut,aBoxOut);
    }


    std::vector<Im2DGen *>   aVIn = aTIn.VecOfIm(aCamIn.Sz());
    ELISE_COPY(aTIn.all_pts(),aTIn.in(),StdOutput(aVIn));
    std::vector<Im2DGen *>   aVOut = aTIn.VecOfIm(aSzOut);

    cKernelInterpol1D * aKern = cKernelInterpol1D::StdInterpCHC(mScale,100);
    int aNbCh = aVIn.size();
    double aSzK = aKern->SzKernel();
    Pt2di aSzIn = aTIn.sz();
    double aTxKer = aSzIn.x - aSzK;
    double aTyKer = aSzIn.y - aSzK;


    int aPas = 4;
    int aSzXR = 1+(aSzOut.x+ aPas-1) / aPas;
    int aSzXY = 1+(aSzOut.y+ aPas-1) / aPas;
    TIm2D<REAL8,REAL8> aTImX(Pt2di(aSzXR+1,aSzXY+1));
    TIm2D<REAL8,REAL8> aTImY(Pt2di(aSzXR+1,aSzXY+1));
    Pt2di aPInd;
    for ( aPInd.x=0; aPInd.x<=aSzXR ; aPInd.x++)
    {
       for (aPInd.y=0; aPInd.y<=aSzXY ; aPInd.y++)
       {
            Pt2dr aPIm = TransfoEpip(Pt2dr(aPInd*aPas),aCamOut,aCamIn);
            aTImX.oset(aPInd,aPIm.x);
            aTImY.oset(aPInd,aPIm.y);
       }
    }



    double UnSPas = 1.0/aPas;
    double aDMax = 0;
    for (int anX=0; anX<aSzOut.x ; anX++)
    {
//  std::cout << "EpiXXX " << aSzOut.x - anX << "\n";
       Pt2dr aPR(anX/double(aPas),0);
       for (int anY=0; anY<aSzOut.y ; anY++)
       {

            Pt2dr aPIm(aTImX.getr(aPR),aTImY.getr(aPR));
            bool Ok =    (aPIm.x > aSzK)
                      && (aPIm.y > aSzK)
                      && (aPIm.x< aTxKer)
                      && (aPIm.y< aTyKer);

            if (0)  // Verification des tabulations
            {
                if (Ok)
                {
                    Pt2dr aPImB = TransfoEpip(Pt2dr(anX,anY),aCamOut,aCamIn);
                    double aD =  euclid(aPImB,aPIm) ;
                    // if (aD > 0.1)
                    if (aD > aDMax)
                    {
                       aDMax = aD;
                       std::cout << "DTtteestt " << aDMax << " " << anX << " " << anY << "\n";
                       // std::cout << "     " <<   aPIm << " " << aPImB << " " << aSzIn << "\n";
                       // getchar();
                    }
                }
            }


            for (int aK=0 ; aK<aNbCh ; aK++)
            {
                double aVal =  Ok ?
                               aKern->Interpole(*(aVIn[aK]),aPIm.x,aPIm.y) :
                               0 ;
                aVOut[aK]->TronqueAndSet(Pt2di(anX,anY),aVal);
                
            }
            aPR.y += UnSPas;
       }
    }
    delete aKern;


   ELISE_COPY
   (
        aTOut.all_pts(),
        StdInPut(aVOut),
        aTOut.out()
   );

#endif



}






/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant à la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA 
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  à l'utilisation,  à la modification et/ou au
développement et à la reproduction du logiciel par l'utilisateur étant 
donné sa spécificité de logiciel libre, qui peut le rendre complexe à 
manipuler et qui le réserve donc à des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités à charger  et  tester  l'adéquation  du
logiciel à leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement, 
à l'utiliser et l'exploiter dans les mêmes conditions de sécurité. 

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez 
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
Footer-MicMac-eLiSe-25/06/2007*/
