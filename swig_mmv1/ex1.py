import mm3d
c=mm3d.CamOrientFromFile("Ori-FishEyeBasic/Orientation-Calibration_geo_14_001_01_015000.thm.dng_G.tif.xml")
p=mm3d.Pt2dr(1000,1000)
prof=1

print('''//   R3 : "reel" coordonnee initiale
//   L3 : "Locale", apres rotation
//   C2 :  camera, avant distortion
//   F2 : finale apres Distortion
//   M2 : coordonnees scannees 
//
//       Orientation      Projection      Distortion      Interne mm
//   R3 -------------> L3------------>C2------------->F2------------>M2
//''')
print("Focale ",c.Focale())
print("M2 ",p," ---> F2 ",c.NormM2C(p)," --->C2 ",c.F2toC2(c.NormM2C(p))," ---> R3 ",c.ImEtProf2Terrain(c.NormM2C(p),prof))
                     
l=[1,0,0,0,1,0,0,0,1]
print(l)
r=mm3d.list2rot(l)
print(r)
ll=mm3d.rot2list(r)
print(ll)

mm3d.createIdealCamXML (100.0, mm3d.Pt2dr(1000,1000), mm3d.Pt2di(2000,2000), "Ori", "Im001", "cam", r, 200.0, 1200)

li = mm3d.getFileSet(".",".*.py")

r=mm3d.quaternion2rot(0.706421209674, 0.000595506000, -0.002847643999, -0.707785709674)
l=mm3d.rot2list(r)


