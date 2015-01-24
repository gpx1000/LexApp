#include <jni.h>
#include <android/log.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#define  LOG_TAG    "main"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

int printOut() {  LOGD("THIS IS A TEST!!!"); }

int generateSoundScape()
{
   /* OpenCV variables */
   cv::Mat gray, frame;

//   int i, j, d=D, ss, key=0;
//   long k=0L, l, ns=2L*(long)(0.5*FS*T), m=ns/N,
//      sso=HIFI?0L:128L, ssm=HIFI?32768L:128L;
//   double **A, a, t, dt=1.0/FS, *w, *phi0, s, y, yp, z, tau1, tau2, x, theta,
//      scale=0.5/sqrt((double)M), q, q2, r, sl, sr, tl, tr, yl, ypl, yr, ypr,
//      zl, zr, hrtf, hrtfl, hrtfr, v=340.0,  /* v = speed of sound (m/s) */
//      hs=0.20;  /* hs = characteristic acoustical size of head (m) */
//   w    = C_ALLOC(M, double);
//   phi0 = C_ALLOC(M, double);
//   A    = C_ALLOC(M, double *);
//   for (i=0; i<M; i++) A[i] = C_ALLOC(N, double);  /* M x N pixel matrix */
//
//   if (!CAM) {  /* Set hard-coded image */
//      for (i=0; i<M; i++) {
//         for (j=0; j<N; j++) {
//            if (BW) { if (P[M-i-1][j]!='#') A[i][j]=WHITE; else A[i][j]=BLACK; }
//            else if (P[i][j]>'a') A[M-i-1][j] = pow(10.0,(P[i][j]-'a'-15)/10.0);  /* 2dB steps */
//            else A[M-i-1][j] = 0.0;
//         }
//      }
//   }
//
//   /* Set lin|exp (0|1) frequency distribution and random initial phase */
//   if (d) for (i=0; i<M; i++) w[i] = TwoPi * FL * pow(1.0* FH/FL,1.0*i/(M-1));
//   else   for (i=0; i<M; i++) w[i] = TwoPi * FL + TwoPi * (FH-FL)   *i/(M-1) ;
//   for (i=0; i<M; i++) phi0[i] = TwoPi * rnd();
//
//   int cam_id = 0;  /* First available OpenCV camera */
//   /* Optionally override ID from command line parameter: prog.exe cam_id */
//   if (argc>1) cam_id = atoi(argv[1]);
//
//   cap.open(cam_id);
//   if (!cap.isOpened()) {
//      fprintf(stderr,"Could not open camera %d\n", cam_id);
//      return 1;
//   }
//   /* Setting standard capture size, may fail; resize later */
//   cap.read(frame);  /* Dummy read needed with some devices */
//   cap.set(CV_CAP_PROP_FRAME_WIDTH , 176);
//   cap.set(CV_CAP_PROP_FRAME_HEIGHT, 144);
//
//   if (VIEW) {  /* Screen views only for debugging */
//      namedWindow("Large", CV_WINDOW_AUTOSIZE);
//      namedWindow("Small", CV_WINDOW_AUTOSIZE);
//   }
//
//   while (key != 27) {  /* Escape key */
//      cap.read(frame);
//
//      if (frame.empty()) {
//         /* Sometimes initial frames fail */
//         fprintf(stderr, "Capture failed\n");
//         key = waitKey((int)(100));
//         continue;
//      }
//
//      Mat tmp;
//      cvtColor(frame,tmp,CV_BGR2GRAY);
//      if (frame.rows != M || frame.cols != N) resize(tmp, gray, Size(N,M)); else gray=tmp;
//
//      if (VIEW) {  /* Screen views only for debugging */
//      /* imwrite("hificodeLarge.jpg", frame); */
//         imshow("Large", frame);
//      /* imwrite("hificodeSmall.jpg", gray); */
//         imshow("Small", gray);
//      }
//
//      key = waitKey((int)(10));
//
//      if (CAM) {  /* Set live camera image */
//         for (i=0; i<M; i++) {
//            for (j=0; j<N; j++) {
//               int mVal=gray.at<uchar>(M-1-i,j)/16;
//               if (mVal == 0) A[i][j]=0;
//               else A[i][j]=pow(10.0,(mVal-15)/10.0);  /* 2dB steps */
//            }
//         }
//      }
//
//      /* Write 8/16-bit mono/stereo .wav file */
//      fp = fopen(FNAME,"wb"); fprintf(fp,"RIFF"); wl(ns*HIST+36L);
//      fprintf(fp,"WAVEfmt "); wl(16L); wi(1); wi(STEREO?2:1); wl(0L+FS);
//      wl(0L+FS*HIST); wi(HIST); wi(HIFI?16:8); fprintf(fp,"data"); wl(ns*HIST);
//      tau1 = 0.5 / w[M-1]; tau2 = 0.25 * tau1*tau1;
//      y = yl = yr = z = zl = zr = 0.0;
//      /* Not optimized for speed */
//      while (k < ns && !STEREO) {
//         if (BSPL) { q = 1.0 * (k%m)/(m-1); q2 = 0.5*q*q; }
//         j = k / m; if (j>N-1) j=N-1; s = 0.0; t = k * dt;
//         if (k < ns/(5*N)) s = (2.0*rnd()-1.0) / scale;  /* "click" */
//         else for (i=0; i<M; i++) {
//            if (BSPL) {  /* Quadratic B-spline for smooth C1 time window */
//               if (j==0) a = (1.0-q2)*A[i][j]+q2*A[i][j+1];
//               else if (j==N-1) a = (q2-q+0.5)*A[i][j-1]+(0.5+q-q2)*A[i][j];
//               else a = (q2-q+0.5)*A[i][j-1]+(0.5+q-q*q)*A[i][j]+q2*A[i][j+1];
//            }
//            else a = A[i][j];  /* Rectangular time window */
//            s += a * sin(w[i] * t + phi0[i]);
//         }
//         yp = y; y = tau1/dt + tau2/(dt*dt);
//         y  = (s + y * yp + tau2/dt * z) / (1.0 + y); z = (y - yp) / dt;
//         l  = sso + 0.5 + scale * ssm * y; /* y = 2nd order filtered s */
//         if (l >= sso-1+ssm) l = sso-1+ssm; if (l < sso-ssm) l = sso-ssm;
//         ss = (unsigned int) l;
//         if (HIFI) wi(ss); else putc(ss,fp);
//         k++;
//      }
//      while (k < ns && STEREO) {
//         if (BSPL) { q = 1.0 * (k%m)/(m-1); q2 = 0.5*q*q; }
//         j = k / m; if (j>N-1) j=N-1;
//         r = 1.0 * k/(ns-1);  /* Binaural attenuation/delay parameter */
//         theta = (r-0.5) * TwoPi/3; x = 0.5 * hs * (theta + sin(theta));
//         tl = tr = k * dt; if (DELAY) tr += x / v; /* Time delay model */
//         x  = fabs(x); sl = sr = 0.0; hrtfl = hrtfr = 1.0;
//         for (i=0; i<M; i++) {
//            if (DIFFR) {
//               /* First order frequency-dependent azimuth diffraction model */
//               if (TwoPi*v/w[i] > x) hrtf = 1.0; else hrtf = TwoPi*v/(x*w[i]);
//               if (theta < 0.0) { hrtfl =  1.0; hrtfr = hrtf; }
//               else             { hrtfl = hrtf; hrtfr =  1.0; }
//            }
//            if (FADE) {
//               /* Simple frequency-independent relative fade model */
//               hrtfl *= (1.0-0.7*r);
//               hrtfr *= (0.3+0.7*r);
//            }
//            if (BSPL) {
//               if (j==0) a = (1.0-q2)*A[i][j]+q2*A[i][j+1];
//               else if (j==N-1) a = (q2-q+0.5)*A[i][j-1]+(0.5+q-q2)*A[i][j];
//               else a = (q2-q+0.5)*A[i][j-1]+(0.5+q-q*q)*A[i][j]+q2*A[i][j+1];
//            }
//            else a = A[i][j];
//            sl += hrtfl * a * sin(w[i] * tl + phi0[i]);
//            sr += hrtfr * a * sin(w[i] * tr + phi0[i]);
//         }
//         if (k < ns/(5*N)) sl = (2.0*rnd()-1.0) / scale;  /* Left "click" */
//         if (tl < 0.0) sl = 0.0;
//         if (tr < 0.0) sr = 0.0;
//         ypl = yl; yl = tau1/dt + tau2/(dt*dt);
//         yl  = (sl + yl * ypl + tau2/dt * zl) / (1.0 + yl); zl = (yl - ypl) / dt;
//         ypr = yr; yr = tau1/dt + tau2/(dt*dt);
//         yr  = (sr + yr * ypr + tau2/dt * zr) / (1.0 + yr); zr = (yr - ypr) / dt;
//         l   = sso + 0.5 + scale * ssm * yl;
//         if (l >= sso-1+ssm) l = sso-1+ssm; if (l < sso-ssm) l = sso-ssm;
//         ss  = (unsigned int) l;
//         if (HIFI) wi(ss); else putc(ss,fp);  /* Left channel */
//         l   = sso + 0.5 + scale * ssm * yr;
//         if (l >= sso-1+ssm) l = sso-1+ssm; if (l < sso-ssm) l = sso-ssm;
//         ss  = (unsigned int) l;
//         if (HIFI) wi(ss); else putc(ss,fp);  /* Right channel */
//         k++;
//      }
//      fclose(fp);
//
//      playSound("hificode.wav");  /* Play the soundscape */
//   /* remove("hificode.wav"); */
//
//      k=0;  /* Reset sample count */
//   }
   return(0);
}