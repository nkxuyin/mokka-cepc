C 
C     *******************************************************
C     *                                                     *
C     *                      Mokka                          * 
C     *   - the detailed geant4 simulation for Tesla -      *
C     *                                                     *
C     * For more information about Mokka, please, go to the *
C     *                                                     *
C     *  polype.in2p3.fr/geant4/tesla/www/mokka/mokka.html  *
C     *                                                     *
C     *    Mokka home page.                                 *
C     *                                                     *
C     *******************************************************
C     
C      $Id: Ex02.f,v 1.12 2006/05/23 11:42:08 musat Exp $
C      $Name: mokka-07-00 $
C     
C      History
C      first implementation for the 
C      Mokka Common Geometry Access (CGA) by 
C      Gabriel Musat (musat@poly.in2p3.fr), March 2003
C     
C      see CGA documentation at 
C      http://polype.in2p3.fr/geant4/tesla/www/mokka/
C             software/doc/CGADoc/CGAIndex.html
C     -------------------------------------------------------
C 
         Program Test CGA

         CHARACTER*30 main, particle
         CHARACTER*30 volName(1000), matName(1000)
	 CHARACTER*30 finalVolName
         CHARACTER*6 model, host, user, passwd, setup
         REAL*8 x0, initial(3), final(3), direction(3), local(3)
         REAL*8 distance(1000), nbx0(1000), prepoints(1000, 3)
         REAL*8 nIntLen(1000), EField(3), BField(3)
	 REAL energy
	 REAL*8 IBdl, IEdl
	 REAL*8 Density, Pressure, Temperature, RadLen, IntLen
	 LOGICAL okflag
	 INTEGER nbp, nsteps

	 okflag=.true.
         model='ILD_00'
         setup=' '
         host=' '
         user=' '
         passwd=' '
         initial(1)=-68
         initial(2)=169
         initial(3)=0
         final(1)=-200*sin(3.1418/8)
         final(2)=200*cos(3.1418/8)
         final(3)=0
 	 direction(1)=final(1)-initial(1)
 	 direction(2)=final(2)-initial(2)
 	 direction(3)=final(3)-initial(3)
	 particle='geantino'
	 energy=20
	 nbp=1
	 nsteps=1000

C      call CGAInit(model, setup host, user, passwd)
       call CGAInit('', 'D09M1', '',
     *   '', '', '')
       call CGABeamOn(initial, final, direction, 
     c                  particle, energy, nbp)
       call CGAWhereAmI(final, finalVolName)
 	 write(*,15) 'volName: ', finalVolName
   15    format(a, 3x, a)
       call CGAGetSteps(volName, matName, distance, prepoints, 
     c                  nbx0, nIntLen, nsteps, okflag)
	 if(okflag .eqv. .true.) then
	   do i=1,nsteps
		write(*,12)i, volName(i), matName(i), distance(i), 
     c          prepoints(i,1), prepoints(i,2), prepoints(i,3), 
     c          nbx0(i), nIntLen(i)
	   enddo
	 endif 

   12    format(i6, 3x, a, 3x, a, 3x, f9.3, 3x, 3(f9.3, 3x), f9.5, f9.5)


	 nsteps=1000
	 okflag=.true.
 	 call CGAGetVolumeData('EndCapLog', distance, prepoints, 
     c			nbx0, nIntLen, nsteps, okflag)
	 write(*, 23) 'nsteps=',nsteps
   23    format(a, i6)
	 if(okflag .eqv. .true.) then
	   do i=1,nsteps
		write(*,17)'EnvLog', distance(i), prepoints(i,1), 
     c          prepoints(i,2), prepoints(i,3), nbx0(i), nIntLen(i)
	   enddo
	 endif 
   17    format(a, 3x, f20.8, 3x, 3(f9.3, 3x), f9.5, f9.5)

         initial(1)=0
         initial(2)=0
         initial(3)=0
         final(1)=100
         final(2)=100
         final(3)=2000
	 call CGAGetBdl(initial, final, IBdl)
	 write(*, 37) 'IBdl=', IBdl

	 call CGAGetEdl(initial, final, IEdl)
	 write(*, 37) 'IEdl=', IEdl
   37    format(a, 3x, f10.5) 

	 call CGAGetB(initial, BField)
	 write(*, 47) 'B in origin: (', BField(1),',',
     *      BField(2),',',BField(3),')'
	 call CGAGetE(initial, EField)
	 write(*, 47) 'E in origin: (', EField(1),',',
     *      EField(2),',',EField(3),')'
   47    format(a,f10.5,a,f10.5,a,f10.5,a)
         final(1)=0
         final(2)=1730
         final(3)=0
         call CGAGetMaterialName(final, finalVolName)
	 call CGAGetDensity(final, Density)
	 call CGAGetPressure(final, Pressure)
	 call CGAGetTemperature(final, Temperature)
	 call CGAGetRadLen(final, RadLen)
	 call CGAGetIntLen(final, IntLen)
 	 write(*,57) 'MatName: ', finalVolName, ' density: ',
     *   Density,' g/cm3, Pressure: ',Pressure,' bar, Temperature: ',
     *   Temperature, ' RadLen: ', RadLen, ' IntLen: ', IntLen
   57    format(3a,f10.5,a,f10.5,a,f10.5,a,f10.5,a,f10.5)

	 call CGAGetListOfLVs(final, volName, nsteps)
	 do i=1,nsteps
		write(*,67) volname(i)
	 enddo

	 call CGAGetListOfPVs(final, volName, nsteps)
	 do i=1,nsteps
		write(*,67) volname(i)
	 enddo
	 call CGAGetRegionName(final, finalVolName)
	 write(*,67), finalVolName
   67    format(a)
	 call CGAGetLocalPosition(final, local)
	 write(*,77)'LocalPosition: (', local(1), ',', local(2),',',
     *   local(3), ')'
   77    format(a,f10.5,a,f10.5,a,f10.5,a)
         initial(1)=0
         initial(2)=0
         initial(3)=0
	 call CGAIsTracker(initial, okflag)
	 if(okflag .eqv. .true.) then
		write(*,77)'Position: (',initial(1),',',
     *          initial(2),',',initial(3),') is in tracker region'	
	 endif 
         final(1)=0
         final(2)=1730
         final(3)=0
	 call CGAIsCalorimeter(final, okflag)
	 if(okflag .eqv. .true.) then
		write(*,77)'Position: (',final(1),',',
     *          final(2),',',final(3),') is in calorimeter region'	
	 endif 
         return
         end

