C $Id: plotl.f,v 1.3 2007/07/24 13:10:18 gdsjaar Exp $
C $Log: plotl.f,v $
C Revision 1.3  2007/07/24 13:10:18  gdsjaar
C Fix problem with boundary condition memory overwrite.
C
C Remove old ls5 and r25 terminal tests
C
C Revision 1.2  1998/07/14 18:19:39  gdsjaar
C Removed unused variables, cleaned up a little.
C
C Changed BLUE labels to GREEN to help visibility on black background
C (indirectly requested by a couple users)
C
C Revision 1.1.1.1  1990/11/30 11:13:44  gdsjaar
C FASTQ Version 2.0X
C
c Revision 1.1  90/11/30  11:13:43  gdsjaar
c Initial revision
c 
C
CC* FILE: [.QMESH]PLOTL.FOR
CC* MODIFIED BY: TED BLACKER
CC* MODIFICATION DATE: 7/6/90
CC* MODIFICATION: COMPLETED HEADER INFORMATION
C
      SUBROUTINE PLOTL (MXND, XN, YN, NXL, KREG, XMIN, XMAX, YMIN,
     &   YMAX, LLL, DEV1)
C***********************************************************************
C
C  SUBROUTINE PLTNXL = PLOTS THE CURRENT MESH FROM THE NXL ARRAY
C
C***********************************************************************
C
      DIMENSION NXL (2, 3 * MXND), XN (MXND), YN (MXND)
      DIMENSION X (2), Y (2)
C
      CHARACTER * 72 DUMMY, HOLD, CDUMMY * 4, DEV1 * 3
C
C  INITIALIZE THE PLOTTING SURFACE
C
      CALL PLTBGN
      XDIMR = XMAX - XMIN
      YDIMR = YMAX - YMIN
      XDIMD = 1.
      YDIMD = .75
      CALL MPVIEW (0., XDIMD, 0., YDIMD)
      XRAT = XDIMR / XDIMD
      YRAT = YDIMR / YDIMD
      IF (XRAT .LT. YRAT) THEN
         XDIMR = XDIMD * YRAT
         XX1 =  (XMIN + XMAX - XDIMR) * .5
         XX2 =  (XMIN + XMAX + XDIMR) * .5
         XDIMR = XX2 - XX1
         YY1 = YMIN
         YY2 = YMAX
      ELSE
         YDIMR = YDIMD * XRAT
         YY1 =  (YMIN + YMAX - YDIMR) * .5
         YY2 =  (YMIN + YMAX + YDIMR) * .5
         YDIMR = YY2 - YY1
         XX1 = XMIN
         XX2 = XMAX
      ENDIF
      XX1 = XX1 -  (XDIMR * .1)
      XX2 = XX2 +  (XDIMR * .1)
      YY1 = YY1 -  (YDIMR * .1)
      YY2 = YY2 +  (YDIMR * .1)
      CALL MPORT2 (XX1, XX2, YY1, YY2)
      CALL PLTFRM (0)
      CALL GETDUM (KREG, HOLD, LEN)
      DUMMY = ' '
      DUMMY (8:7 + LEN) = HOLD (1:LEN)
      DUMMY (1:7) = 'REGION '
      LEN = LEN + 7
      CALL PLTXTH (XDIMD * .05, YDIMD * .95, DUMMY (1:LEN))
C
C  PLOT THE LINES IN NXL ARRAY,  SKIPPING DELETIONS
C
      DO 100 I = 1, LLL
         IF (NXL (1, I) .GT. 0) THEN
            X (2) = XN (NXL (2, I))
            Y (2) = YN (NXL (2, I))
            X (1) = XN (NXL (1, I))
            Y (1) = YN (NXL (1, I))
            CALL MPD2VC (1, X (1), Y (1), X (2), Y (2))
         ENDIF
  100 CONTINUE
C
      CALL PLTFLU
C
      RETURN
C
      END
