//< and > logic

    //Get subsequent args:
       //0) Save position of command; use it to get position of following args
       //1) Check for < char
       //   If present, note position
       //2) Check for > char
       //   If present, note position
       //3) If both < & >, see which is first
       //4) 
       
       //First we'll check for read/write chars
       char* readCharLoc = strchr(cmdLineBuffer, '<');
       char* writeCharLoc = strchr(cmdLineBuffer, '>');

       //Check for presence of & char
       char* ampersand = strchr(cmdLineBuffer, '&');

       //If both are present, we need to see which is first
       if (readCharLoc && writeCharLoc) {
           int firstCharDist = 0;
           readCharDist = 0;
           writeCharDist = 0;
           printf("Both present\n");

           readCharDist = strcspn(cmdLineBuffer, "<");
           writeCharDist =  strcspn(cmdLineBuffer, ">");

           printf("<: %d\n", readCharDist);
           printf(">: %d\n", writeCharDist);

           //Get the smallest distance; assign to firstCharDist
           if (readCharDist < writeCharDist) {
               firstCharDist = readCharDist;
           }
           else {
               firstCharDist = writeCharDist;
           
        char firstHalf[2048];
        char secondHalf[2048];
        memcpy(firstHalf, "\0", 2048);
        memcpy(secondHalf, "\0", 2048);
           }
        
       }
