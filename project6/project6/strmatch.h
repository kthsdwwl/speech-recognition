#include <stdio.h>
#include <string.h>
#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define max_str_len 50 


int dp[max_str_len][max_str_len];
int arrow[max_str_len][max_str_len];

int correctChar(char *s1, char *s2) {
    int x, y, s1len, s2len;
    s1len = strlen(s1);
    s2len = strlen(s2);
    dp[0][0] = 0;
    for (x = 1; x <= s2len; ++x) {
        dp[x][0] = dp[x-1][0] + 1;
		arrow[x][0] = 0;
	}
    for (y = 1; y <= s1len; ++y) {
        dp[0][y] = dp[0][y-1] + 1;
		arrow[0][y] = 1;
	}
    for (x = 1; x <= s2len; ++x)
        for (y = 1; y <= s1len; ++y) {
			int a = dp[x-1][y] + 1, b = dp[x][y-1] + 1, c = dp[x-1][y-1] + (s1[y-1] == s2[x-1] ? 0 : 1);
			if (a < b && a < c) {
				dp[x][y] = a;
				arrow[x][y] = 0;
			} else if (b < a && b < c) {
				dp[x][y] = b;
				arrow[x][y] = 1;
			} else {
				dp[x][y] = c;
				arrow[x][y] = 2;
			}
		}
 
    //return dp[s2len][s1len];
	x = s2len; y = s1len;
	int insert = 0, dele = 0, replace = 0;
	while (x != 0 || y != 0) {
		if (arrow[x][y] == 0) {
			--x;
			++insert;
		} else if (arrow[x][y] == 1) {
			--y;
			++dele;
		} else {
			--x; --y;
		}
	}
	replace = dp[s2len][s1len] - insert - dele;

	return s1len - dele - replace;
	//return s2len - insert + dele - replace;
}