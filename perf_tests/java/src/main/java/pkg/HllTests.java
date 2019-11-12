package pkg;

import org.openjdk.jmh.annotations.*;
import java.util.concurrent.TimeUnit;
import java.util.*;
import java.lang.*;
import com.facebook.stats.cardinality.*;
import com.google.common.hash.Hashing;


@BenchmarkMode(Mode.AverageTime)
@OutputTimeUnit(TimeUnit.NANOSECONDS)
@State(Scope.Thread)
@Warmup(iterations = 10, time = 200, timeUnit = TimeUnit.NANOSECONDS)
@Measurement(iterations = 20, time = 200, timeUnit = TimeUnit.NANOSECONDS)
public class HllTests
{
    public static final int           COUNT         = 10000;
    public static final int           STRING_LENGTH = 256;

    private static final List<String> strs          = new ArrayList<>();
    private static final List<Long>   hashes        = new ArrayList<>();
    private static final List<Long>   hashes2       = new ArrayList<>();
    private static final List<Long>   hashesLarge       = new ArrayList<>();

    public static void merge(HyperLogLog log1, HyperLogLog log2)
    {
        for ( int registerIx = 0; registerIx < log1.buckets().length; ++registerIx )
        {
            log1.buckets()[registerIx] = Math.max( log1.buckets()[registerIx], log2.buckets()[registerIx] );
        }
    }

    public static String generateString()
    {
        int leftLimit = 97; // letter 'a'
        int rightLimit = 122; // letter 'z'
        Random random = new Random();
        int targetStringLength = ((Math.abs( random.nextInt() )) % STRING_LENGTH) + 10;
        StringBuilder buffer = new StringBuilder( targetStringLength );

        for ( int i = 0; i < targetStringLength; i++ )
        {
            int randomLimitedInt = leftLimit + (int) (random.nextFloat() * (rightLimit - leftLimit + 1));
            buffer.append( (char) randomLimitedInt );
        }
        return buffer.toString();
    }

    static
    {
        for ( int i = 0; i < COUNT; ++i )
        {
            strs.add( generateString() );
            hashes.add( Hashing.murmur3_128().hashString( generateString(), java.nio.charset.Charset.defaultCharset() ).asLong() );
            hashes2.add( Hashing.murmur3_128().hashString( generateString(), java.nio.charset.Charset.defaultCharset() ).asLong() );
        }

        for ( int i = 0; i < COUNT * 10; ++i )
        {
            hashesLarge.add( Hashing.murmur3_128().hashString( generateString(), java.nio.charset.Charset.defaultCharset() ).asLong() );
        }
    }

    @State(Scope.Benchmark)
    public static class ExecutionPlan
    {
        public int         iteration  = 0;
        int                buckets    = 1 << 14;
        public HyperLogLog estimator  = new HyperLogLog( buckets );
        public HyperLogLog estimator2 = new HyperLogLog( buckets );

        @Setup(Level.Invocation)
        public void setUp()
        {
            for ( final Long hash : hashes )
            {
                estimator.add( hash );
                estimator2.add( hash );
            }
        }
    }

    @Benchmark
    @Fork(1)
    public void AddString(ExecutionPlan plan)
    {
        plan.estimator.add( Hashing.murmur3_128().hashString( strs.get( plan.iteration ), java.nio.charset.Charset.defaultCharset() ).asLong() );
        plan.iteration = (plan.iteration >= (COUNT - 1)) ? 0 : plan.iteration + 1;
    }

    @Benchmark
    @Fork(1)
    public void AddHash(ExecutionPlan plan)
    {
        plan.estimator.add( hashes.get( plan.iteration ) );
        plan.iteration = (plan.iteration >= (COUNT - 1)) ? 0 : plan.iteration + 1;
    }

    @Benchmark
    @Fork(1)
    public void Cardinality(ExecutionPlan plan)
    {
        plan.estimator.estimate();
    }

    @Benchmark
    @Fork(1)
    public void Merge(ExecutionPlan plan)
    {
        merge( plan.estimator, plan.estimator2 );
    }

    @Benchmark
    @Fork(1)
    public void FullTest(ExecutionPlan plan)
    {
        HyperLogLog estimator  = new HyperLogLog( 1 << 14 );
        for ( Long hash : hashesLarge ) {
            estimator.add( hash );
        }
        estimator.estimate();
    }
}
