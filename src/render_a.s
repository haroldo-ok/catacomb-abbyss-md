/* render_a.s - 68000 inner loops for the 3-D view */

/* View buffer layout: 40 tile columns of 480 bytes; inside a tile column */
/* row y is at y*4, and byte (col&3) holds two pixels of a 2-pixel column. */

        .text
        .align 2

/* void R_FillSkyGround(u8 *buf, u32 sky, u32 ground) */
/* rows 0..59 = sky, 60..119 = ground in every tile column */
        .globl R_FillSkyGround
R_FillSkyGround:
        movem.l %d2-%d7/%a2-%a6,-(%sp)
        move.l  48(%sp),%a6
        move.l  52(%sp),%d0
        move.l  %d0,%d1
        move.l  %d0,%d2
        move.l  %d0,%d3
        move.l  %d0,%d4
        move.l  %d0,%d5
        move.l  %d0,%d6
        move.l  %d0,%d7
        move.l  %d0,%a0
        move.l  %d0,%a1
        move.l  %d0,%a2
        move.l  %d0,%a3
        move.l  %a6,%a4
        move.w  #39,-(%sp)
1:
        movem.l %d0-%d7/%a0-%a3,0(%a4)
        movem.l %d0-%d7/%a0-%a3,48(%a4)
        movem.l %d0-%d7/%a0-%a3,96(%a4)
        movem.l %d0-%d7/%a0-%a3,144(%a4)
        movem.l %d0-%d7/%a0-%a3,192(%a4)
        lea     480(%a4),%a4
        subq.w  #1,(%sp)
        bpl.s   1b
        move.l  58(%sp),%d0
        move.l  %d0,%d1
        move.l  %d0,%d2
        move.l  %d0,%d3
        move.l  %d0,%d4
        move.l  %d0,%d5
        move.l  %d0,%d6
        move.l  %d0,%d7
        move.l  %d0,%a0
        move.l  %d0,%a1
        move.l  %d0,%a2
        move.l  %d0,%a3
        lea     240(%a6),%a4
        move.w  #39,(%sp)
2:
        movem.l %d0-%d7/%a0-%a3,0(%a4)
        movem.l %d0-%d7/%a0-%a3,48(%a4)
        movem.l %d0-%d7/%a0-%a3,96(%a4)
        movem.l %d0-%d7/%a0-%a3,144(%a4)
        movem.l %d0-%d7/%a0-%a3,192(%a4)
        lea     480(%a4),%a4
        subq.w  #1,(%sp)
        bpl.s   2b
        addq.l  #2,%sp
        movem.l (%sp)+,%d2-%d7/%a2-%a6
        rts

/* void R_DrawWalls(const WallCol *cols, u8 *buf) */
/* WallCol = { const u8 *tex; u16 scale; u8 u, side; u8 *hitp; }  x 160 */
/* Columns are drawn in pairs: when both columns of a pair use the same
 * texture column and their heights differ by at most one row step, a word
 * scaler draws both at once, else two byte scalers are called. */
        .globl R_DrawWalls
R_DrawWalls:
#ifdef PROFILE
        clr.w   wall_wpairs
#endif
        movem.l %d2-%d7/%a2-%a6,-(%sp)
        move.l  48(%sp),%a2
        move.l  52(%sp),%a3
        lea     scaler_table,%a4
        lea     wscaler_table,%a6
        moveq   #79,%d5
        moveq   #0,%d4                  /* byte offset in the long: 0 / 2 */
1:
        move.l  %a3,%a1
        add.w   %d4,%a1
        move.l  (%a2),%a0
        move.w  4(%a2),%d1
        move.w  16(%a2),%d2
        cmp.l   12(%a2),%a0
        beq.s   7f
        /* close walls (texels >= 3 columns wide): pair columns of one face */
        cmp.w   #96,%d1
        bcs.s   3f
        cmp.w   #96,%d2
        bcs.s   3f
        move.l  8(%a2),%d3
        beq.s   3f
        cmp.l   20(%a2),%d3
        bne.s   3f
        move.b  7(%a2),%d3
        cmp.b   19(%a2),%d3
        bne.s   3f
        bra.s   8f
7:      move.w  %d2,%d3
        sub.w   %d1,%d3
        addq.w  #1,%d3
        cmp.w   #2,%d3
        bhi.s   3f
8:      cmp.w   %d1,%d2                 /* use the taller of the two */
        bls.s   2f
        move.w  %d2,%d1
2:      tst.w   %d1
        beq.s   5f
        add.w   %d1,%d1
        add.w   %d1,%d1
        move.l  0(%a6,%d1.w),%a5
#ifdef PROFILE
        addq.w  #1,wall_wpairs
#endif
        jsr     (%a5)
        bra.s   5f
3:      /* two separate columns */
        tst.w   %d1
        beq.s   4f
        add.w   %d1,%d1
        add.w   %d1,%d1
        move.l  0(%a4,%d1.w),%a5
        jsr     (%a5)
4:      addq.l  #1,%a1
        tst.w   %d2
        beq.s   5f
        move.l  12(%a2),%a0
        add.w   %d2,%d2
        add.w   %d2,%d2
        move.l  0(%a4,%d2.w),%a5
        jsr     (%a5)
5:      lea     24(%a2),%a2
        eor.w   #2,%d4
        bne.s   6f
        lea     480(%a3),%a3
6:      dbra    %d5,1b
        movem.l (%sp)+,%d2-%d7/%a2-%a6
        rts

/* void R_ScaleColumn(const u8 *seg, u8 *dest, const u8 *rowsrc, const s16 *starts) */
/* Draws one sprite column made of segments {top,count,pixels...} 0xFF end. */
/* rowsrc[r] = source row for view row r (255 = none), starts[src] = first */
/* view row of source row src (may be <0 or >=120). */
        .globl R_ScaleColumn
R_ScaleColumn:
        movem.l %d2-%d5/%a2-%a6,-(%sp)
        move.l  40(%sp),%a0        /* seg */
        move.l  44(%sp),%a1        /* dest column */
        move.l  48(%sp),%a2        /* rowsrc */
        move.l  52(%sp),%a3        /* starts */
        moveq   #0,%d2
1:
        moveq   #0,%d0
        move.b  (%a0)+,%d0        /* top */
        cmp.b   #0xFF,%d0
        beq.s   9f
        moveq   #0,%d1
        move.b  (%a0)+,%d1        /* count */
        move.l  %a0,%a4
        sub.l   %d0,%a4        /* a4 = pixels - top */
        move.w  %d0,%d3
        add.w   %d3,%d3
        move.w  0(%a3,%d3.w),%d4        /* first row */
        move.w  %d0,%d3
        add.w   %d1,%d3
        add.w   %d3,%d3
        move.w  0(%a3,%d3.w),%d5        /* end row */
        add.l   %d1,%a0        /* skip pixels */
        tst.w   %d4
        bpl.s   2f
        moveq   #0,%d4
2:
        cmp.w   #120,%d5
        ble.s   3f
        move.w  #120,%d5
3:
        cmp.w   %d5,%d4
        bge.s   1b
        lea     0(%a2,%d4.w),%a5        /* rowsrc + first row */
        move.w  %d4,%d3
        add.w   %d3,%d3
        add.w   %d3,%d3        /* d3 = row*4 */
        lea     0(%a1,%d3.w),%a6
        sub.w   %d4,%d5
        subq.w  #1,%d5
4:
        move.b  (%a5)+,%d2              /* src row */
        move.b  0(%a4,%d2.w),(%a6)
        addq.w  #4,%a6
        dbra    %d5,4b
        bra.s   1b
9:
        movem.l (%sp)+,%d2-%d5/%a2-%a6
        rts

/* void R_CastCol(u16 c)
 * Casts the ray of 2-pixel screen column c from the camera point and fills
 * wallcols[c] (WallCol: 0 tex, 4 scale, 6 u, 7 side, 8 hit pointer) and
 * zbuffer[c].  Per frame parameters come from cast_frame:
 *   0 fine angle, 2 fxp, 4 fxn, 6 fyp, 8 fyn (u16 fractions to the next
 *   tile edge), 10 viewx low word, 12 viewy low word, 16 tile pointer of
 *   the camera tile, 20 light wall table, 24 dark wall table (u16[tile]).
 * The DDA marks spotvis (tilemap+2048) and stops at the first solid tile
 * whose entry point lies beyond the view plane (entry * cos >= 1.0). */
        .globl R_CastCol_
R_CastCol_:
        movem.l %d2-%d7/%a2-%a5,-(%sp)
        move.w  46(%sp),%d0             /* c */
        add.w   %d0,%d0
        move.w  %d0,%a4                 /* a4 = c*2 */
        move.w  %d0,%d1
        add.w   %d1,%d1                 /* c*4 */
        add.w   %d1,%d0                 /* c*6 */
        add.w   %d0,%d0                 /* c*12 */
        lea     wallcols,%a5
        add.w   %d0,%a5
        lea     col_angle,%a0
        move.w  0(%a0,%a4.w),%d1
        add.w   cast_frame,%d1
        bpl.s   1f
        add.w   #3600,%d1
        bra.s   2f
1:      cmp.w   #3600,%d1
        blt.s   2f
        sub.w   #3600,%d1
2:      add.w   %d1,%d1
        add.w   %d1,%d1
        move.w  %d1,%a2                 /* a2 = angle*4 */
        lea     ray_invcos,%a0
        move.l  0(%a0,%d1.w),%d4        /* dX */
        lea     ray_invsin,%a0
        move.l  0(%a0,%d1.w),%d5        /* dY */
        lea     ray_cos,%a0
        move.l  0(%a0,%d1.w),%d2
        lea     ray_sin,%a0
        move.l  0(%a0,%d1.w),%d3

        /* x: sideX = frac * dX */
        tst.l   %d2
        ble.s   3f
        move.w  #32,%a1
        move.w  cast_frame+2,%d0
        bra.s   4f
3:      move.w  #-32,%a1
        move.w  cast_frame+4,%d0
4:      move.l  %d4,%d6
        swap    %d6
        mulu.w  %d0,%d6
        mulu.w  %d4,%d0
        clr.w   %d0
        swap    %d0
        add.l   %d6,%d0
        cmp.l   #0x00FFFFFF,%d4
        bcs.s   5f
        move.l  #0x7FFFFFFF,%d0
5:      move.l  %d0,%d2                 /* sideX */

        /* y grows to the south: direction y = -sin */
        tst.l   %d3
        bge.s   6f
        moveq   #1,%d7
        move.w  cast_frame+6,%d0
        bra.s   7f
6:      moveq   #-1,%d7
        move.w  cast_frame+8,%d0
7:      move.l  %d5,%d6
        swap    %d6
        mulu.w  %d0,%d6
        mulu.w  %d5,%d0
        clr.w   %d0
        swap    %d0
        add.l   %d6,%d0
        cmp.l   #0x00FFFFFF,%d5
        bcs.s   8f
        move.l  #0x7FFFFFFF,%d0
8:      move.l  %d0,%d3                 /* sideY */

        move.l  cast_frame+16,%a0
        move.b  #1,2048(%a0)
        lea     col_cos,%a3
        add.w   %a4,%a3
        move.w  #95,%d1
10:
        cmp.l   %d2,%d3
        bls.s   12f
        move.l  %d2,%d0
        add.l   %d4,%d2
        adda.w  %a1,%a0
        tst.b   (%a0)
        bne.s   11f
        move.b  #1,2048(%a0)
        dbra    %d1,10b
        bra     nohit
11:     /* solid tile entered on an x step */
        move.l  %d2,%d6
        cmp.l   %d3,%d6
        bls.s   13f
        move.l  %d3,%d6
13:     lsr.l   #8,%d6
        mulu.w  (%a3),%d6
        cmp.l   #0x01000000,%d6
        bcc.s   hitx
        dbra    %d1,10b
        bra     nohit
12:
        move.l  %d3,%d0
        add.l   %d5,%d3
        adda.w  %d7,%a0
        tst.b   (%a0)
        bne.s   14f
        move.b  #1,2048(%a0)
        dbra    %d1,10b
        bra     nohit
14:     /* solid tile entered on a y step */
        move.l  %d2,%d6
        cmp.l   %d3,%d6
        bls.s   15f
        move.l  %d3,%d6
15:     lsr.l   #8,%d6
        mulu.w  (%a3),%d6
        cmp.l   #0x01000000,%d6
        bcc   hity
        dbra    %d1,10b
        bra     nohit

/* x side (east / west face): u from the y coordinate of the hit */
hitx:
        move.b  #1,2048(%a0)
        moveq   #0,%d2
        move.b  (%a0),%d2
        cmp.w   #71,%d2                 /* NUMFLOORS */
        bcc     nohit
        move.l  %a0,8(%a5)
        move.b  #1,7(%a5)
        bsr     castscale               /* d3 = s8, scale stored */
        lea     ray_sin,%a1
        move.l  0(%a1,%a2.w),%d5
        move.w  cast_frame+12,%d6
        tst.l   %d5
        bmi.s   1f
        mulu.w  %d5,%d3
        lsr.l   #8,%d3
        sub.w   %d3,%d6
        bra.s   2f
1:      neg.l   %d5
        mulu.w  %d5,%d3
        lsr.l   #8,%d3
        add.w   %d3,%d6
2:      moveq   #10,%d0
        lsr.w   %d0,%d6                 /* u = (hit >> 10) & 63 */
        lea     ray_cos,%a1
        tst.l   0(%a1,%a2.w)
        bgt.s   3f
        moveq   #63,%d0                 /* xinc < 0: mirror */
        sub.w   %d6,%d0
        move.w  %d0,%d6
3:      add.w   %d2,%d2
        move.l  cast_frame+20,%a1       /* light first */
        move.w  0(%a1,%d2.w),%d0
        cmp.w   #192,%d0                /* FIRSTWALLPIC */
        bcc.s   castout
        move.l  cast_frame+24,%a1
        move.w  0(%a1,%d2.w),%d0
        bra.s   castout

/* y side (north / south face): u from the x coordinate of the hit */
hity:
        move.b  #1,2048(%a0)
        moveq   #0,%d2
        move.b  (%a0),%d2
        cmp.w   #71,%d2
        bcc   nohit
        move.l  %a0,8(%a5)
        clr.b   7(%a5)
        bsr   castscale
        lea     ray_cos,%a1
        move.l  0(%a1,%a2.w),%d5
        move.w  cast_frame+10,%d6
        tst.l   %d5
        bmi.s   1f
        mulu.w  %d5,%d3
        lsr.l   #8,%d3
        add.w   %d3,%d6
        bra.s   2f
1:      neg.l   %d5
        mulu.w  %d5,%d3
        lsr.l   #8,%d3
        sub.w   %d3,%d6
2:      moveq   #10,%d0
        lsr.w   %d0,%d6
        tst.w   %d7                     /* yinc > 0: mirror */
        ble.s   3f
        moveq   #63,%d0
        sub.w   %d6,%d0
        move.w  %d0,%d6
3:      add.w   %d2,%d2
        move.l  cast_frame+24,%a1       /* dark first */
        move.w  0(%a1,%d2.w),%d0
        cmp.w   #192,%d0
        bcc.s   castout
        move.l  cast_frame+20,%a1
        move.w  0(%a1,%d2.w),%d0

/* d0 = wall picture, d6 = u */
castout:
        move.b  %d6,6(%a5)
        sub.w   #192,%d0
        and.l   #0xFFFF,%d0
        moveq   #13,%d1
        lsl.l   %d1,%d0                 /* 8192 bytes per wall picture */
        and.w   #63,%d6
        lsl.w   #7,%d6                  /* 128 bytes per texture column */
        add.w   %d6,%d0
        add.l   #wall_tex,%d0
        move.l  %d0,(%a5)
        movem.l (%sp)+,%d2-%d7/%a2-%a5
        rts

nohit:
        clr.w   4(%a5)
        clr.l   8(%a5)
        lea     zbuffer,%a1
        clr.w   0(%a1,%a4.w)
        movem.l (%sp)+,%d2-%d7/%a2-%a5
        rts

/* d0 = distance to the entry point (16.16): returns d3 = s8 = d0 >> 8
 * (min 256), stores scale = col_k[c] / s8 / 2 (max 160) */
castscale:
        lsr.l   #8,%d0
        cmp.l   #256,%d0
        bcc.s   1f
        move.l  #256,%d0
1:      move.l  %d0,%d3
        move.w  %a4,%d1
        add.w   %d1,%d1
        lea     col_k,%a1
        move.l  0(%a1,%d1.w),%d4
        divu.w  %d3,%d4
        lsr.w   #1,%d4
        cmp.w   #160,%d4
        bls.s   2f
        move.w  #160,%d4
2:      move.w  %d4,4(%a5)
        lea     zbuffer,%a1
        move.w  %d4,0(%a1,%a4.w)
        rts


        .bss
        .globl wall_wpairs
        .align 2
wall_wpairs: .space 2
