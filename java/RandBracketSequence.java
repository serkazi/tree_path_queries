/*
 */
import java.io.*;
import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
import java.util.Random;

public class RandBracketSequence {
	//private ThreadLocalRandom thr = ThreadLocalRandom.current();
	Random thr = new Random(System.currentTimeMillis());
	private static final int N = 30;
	private static final long OO= Long.MAX_VALUE;
	private static final char OPENING_BRACKET = '(', CLOSING_BRACKET = ')';
	private static final long [][]C = new long[2*N][2*N];
	private static final long []Cat = new long[N];

	static {
		int i,j;
		for ( i = 0; i < N*2; C[i++][0] = 1L ) ;
		for ( i = 1; i < N*2; ++i )
			for ( j = 1; j <= i; ++j )
				C[i][j] = C[i-1][j]+C[i-1][j-1];
		for ( i = 2; i < N; ++i )
			Cat[i] = C[2*i][i]/(i+1);
	}

	/* Knuth's algorithm for choosing "n" objects from {1,2,...,N} 
	 * in our case will be used to choose n objects from {1,2,...,2n}
	 * */
	private int [] randSubset( int N, int n ) {
		int []res = new int[n];
		for ( int m = 0, t = 0; m < n; ++t ) 
			if ( (N-t)*thr.nextDouble() < n-m ) 
				res[m++] = t;
		return res;
	}
	String inverse( String w ) {
		StringBuilder sb = new StringBuilder();
		for ( int i = 0; i < w.length(); ++i )
			sb.append(w.charAt(i)==OPENING_BRACKET?CLOSING_BRACKET:OPENING_BRACKET);
		return sb.toString();
	}
	private boolean isBalanced( String w ) {
		int i,balance = 0;
		for ( i = 0; i < w.length(); ++i )
			balance += (w.charAt(i)==OPENING_BRACKET?1:-1);
		return balance == 0;
	}

	private class BitVector {
		int n;
		int []map;
		BitVector( int n ) {
			this.n= n;
			map= new int[(n>>5)+8];
		}
		void setVal( int pos ) {
			map[pos>>5]|= (1<<(pos&31));
		}
		int access( int pos ) {
			return (map[pos>>5]&(1<<(pos&31)))==0?0:1;
		}
	}

	private void Phi( char []w, int left, int right, StringBuilder sb ) {
		if ( w == null || left > right )
			return ;
		int n = right-left+1,partial_sum = 0,i,j,k,r = 0;
		for ( i = left; i <= right && r == 0; ++i ) 
			if ( (partial_sum += (w[i]==OPENING_BRACKET?1:-1)) == 0 )
				r = i+1;
		assert r > 0: Arrays.toString(w);
		if ( w[left] == OPENING_BRACKET ) {
			for ( i = left; i < r; ++i )
				sb.append(w[i]);
			Phi(w,r,right,sb);
			return ;
		}
		assert w[left] == CLOSING_BRACKET;
		assert w[r-1] == OPENING_BRACKET;
		sb.append(OPENING_BRACKET);
		Phi(w,r,right,sb);
		sb.append(CLOSING_BRACKET);
		for ( i = left+1; i <= r-2; ++i )
			sb.append(w[i]==OPENING_BRACKET?CLOSING_BRACKET:OPENING_BRACKET);
	}

	private void explicitStackPhi( char []w, char []sb ) {
		if ( w == null ) return ;

		int n= w.length/2;

		int cur= 0;
		Deque<Integer> ls= new LinkedList<>(), rs= new LinkedList<>();
		Deque<Long> postAction= new LinkedList<>();
		Deque<Boolean> status= new LinkedList<>();

		for ( ls.addLast(0), rs.addLast(2*n-1), status.addLast(Boolean.FALSE), postAction.addLast(+OO); !ls.isEmpty(); ) {
			int left= ls.pollLast(), right= rs.pollLast(), partial_sum= 0,i,j,k,r= 0;
			boolean done= status.pollLast();
			long action= postAction.pollLast();

			if ( done ) {
				if ( action < +OO ) {
					j= (int)(action>>31);
					i= (int)(action & ((1L<<31)-1L));
					sb[cur++]= CLOSING_BRACKET;
					for ( k= i+1; k <= j-2; ++k )
						sb[cur++]= w[k]==OPENING_BRACKET?CLOSING_BRACKET:OPENING_BRACKET;
				}
				continue ;
			}

			ls.addLast(left); rs.addLast(right); status.addLast(Boolean.TRUE); 
			postAction.addLast(action);

			for ( i= left; i <= right && r == 0; ++i )
				if ( (partial_sum += (w[i]==OPENING_BRACKET?1:-1)) == 0 )
					r = i+1;

			if ( left > right ) continue ;

			assert r > 0: Arrays.toString(w);
			if ( w[left] == OPENING_BRACKET ) {
				for ( i = left; i < r; ++i )
					sb[cur++]= w[i];
				ls.addLast(r); rs.addLast(right);
				status.addLast(false);
				postAction.addLast(+OO);
				continue ;
			}
			assert w[left] == CLOSING_BRACKET;
			assert w[r-1] == OPENING_BRACKET;
			sb[cur++]= OPENING_BRACKET;
			ls.addLast(r); rs.addLast(right); status.addLast(false);
			postAction.addLast( ((long)left) | (((long)r) << 31) );
		}
	}

	public char []randomBPS( int n ) {
		int []L = randSubset(2*n,n);
		char []x = new char[2*n];
		assert L.length == n;
		int i,k;
		for ( i = 0, k = 0; k < n; x[i++] = OPENING_BRACKET, ++k ) 
			for (;i < 2*n && i < L[k]; x[i++] = CLOSING_BRACKET ) ;
		assert k == n;
		for ( ;i < 2*n; x[i++] = CLOSING_BRACKET ) ;
		char []sb= new char[2*n];
		//Phi(x,0,2*n-1,sb);
		explicitStackPhi(x,sb);
		return sb;
	}

	public static void main( String [] args ) throws Exception {
		if ( args.length == 0 ) 
			new RandBracketSequence().go();
		else {
			int n = Integer.parseInt(args[0]), m = Integer.parseInt(args[2]), sigma = Integer.parseInt(args[1]);
			new RandBracketSequence().generate(n,sigma,m);
		}
	}
	private boolean isCBS( char []w ) {
		if ( w == null || w.length == 0 )
			return true ;
		int i,j,k,s = 0,n = w.length;
		for ( i = 0; i < n; ++i ) 
			if ( (s += w[i]==OPENING_BRACKET?1:-1) < 0 )
				return false;
		return s == 0;
	}
	private double chiStatistic( Map<String,Integer> cnt, double expected ) {
		double res = 0, total = 0;
		for ( Map.Entry<String,Integer> entry: cnt.entrySet() )
			total += entry.getValue();
		for ( Map.Entry<String,Integer> entry: cnt.entrySet() )
			res += (entry.getValue()-expected)*(entry.getValue()-expected)/expected;
		return res;
	}
	void generate( int n, int sigma, int m ) throws Exception {
		int C = (int)Math.sqrt(n);
		PrintWriter writer= new PrintWriter(System.out,false);
		for ( int i = 0; i < m; ++i ) {
			char []str= randomBPS(n);
			assert str.length == 2*n;
			assert isCBS(str);
			writer.write('(');
			writer.write(str);
			writer.write(")\n");
			for ( int j = 0; j <= n; ++j ) {
				writer.print(thr.nextInt(sigma)+1);
				writer.write(' ');
			}
			writer.write('\n');
			writer.flush();
		}
		writer.close();
	}
	void go() throws Exception {
		for ( int n = 2; n <= 13; ++n ) {
			Map<String,Integer> cnt = new HashMap<>();
			for ( int i = 0; i < (1<<21); ++i ) {
				char []r = randomBPS(n);
				assert isCBS(r);
				/*
				if ( cnt.containsKey(r) )
					cnt.put(r,cnt.get(r)+1);
				else cnt.put(r,1);
				*/
			}
			/*for ( Map.Entry<String,Integer> entry: cnt.entrySet() )
				System.out.printf("%s: %d\n",entry.getKey(),entry.getValue());*/
			System.out.printf("%d %f %d\n",n,chiStatistic(cnt,(1<<21)/(Cat[n]+0.00)),Cat[n]-1);
		}
	}
};

