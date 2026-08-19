#include<benchmark/benchmark.h>
#include "util.h"
#include<vector>
#include<string>
#include<random>
// Create a test directory structure
class ComparisonBenchmark : public benchmark::Fixture
{
public : void SetUp( const ::benchmark::State &)override
{
    // Create test directories with mock files
    testDir1 = L "C:\\Temp\\TestDir1";
    testDir2 = L "C:\\Temp\\TestDir2";
// Note: In real scenario, we'd create actual test files
// For now, this benchmark structure shows the performance characteristics
}

void TearDown( const ::benchmark::State &)override
{
// Cleanup would happen here
}

protected : WCHAR testDir1[MAX_PATH];
WCHAR testDir2[MAX_PATH]; }
;
// Benchmark the file listing and comparison
BENCHMARK_F(ComparisonBenchmark, FastCompareLargeDirectory)(benchmark::State & state)
{
    for (auto _ :  state)
    {
    // This would benchmark FastCompare with a large directory
    // FastCompare(testDir1, testDir2);
    }
}