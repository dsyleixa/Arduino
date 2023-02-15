// Knuth MMIX
// n = UINT64_C(6364136223846793005) * n + UINT64_C(1442695040888963407);
    

// L'Ecuyer (with prime addend)
// n = UINT64_C(2862933555777941757) * n + UINT64_C(3037000493);


uint64_t  PRN;
#define   HWNOISE ((analogRead(A0)+1)>>32)


uint32_t randk() { // Knuth
   PRN = 6364136223846793005ULL * PRN + 1442695040888963407ULL;
   return (uint32_t)(PRN>>32);
}

uint32_t rande() { // L'Ecuyer
   PRN = 2862933555777941757ULL * PRN + 3037000493ULL;
   return (uint32_t)(PRN>>32);
}




uint64_t srands() {
    PRN = ( (uint64_t)millis() * HWNOISE );
	return prn;
}


//...
//  uint32_t  r, range;  // prn in a range 0...range-1
//  r = n%range;