# Algorithm analysis — Stereo matching (Issue 2.1)

Version: 1.0  
Author: Generated summary for code review  
Date: décembre 02, 2025

## Purpose
This document summarizes a top-down review of the stereo-matching pipeline implemented in the codebase. It documents the 9-stage processing flow, data dependencies, memory allocation map, complexity estimates, parallelization candidates and a recommended testing strategy.

## High-level pipeline (9 stages)
1. Input YUV frames (left and right)
2. YUV -> RGB conversion (yuv2rgb)
3. RGB -> Grayscale conversion (rgb2Gray)
4. Census transform (census)
5. Compute spatial weights (computeWeights)
6. Cost construction per disparity (costConstruction)
7. Cost aggregation with weighted neighborhood (aggregateCosts)
8. Disparity selection / accumulation across disparity range (disparitySelect)
9. Median filter / post-processing (medianFilter) and final output (MD5 or write)

Note: "main.c" orchestrates the loop: buffer allocation, per-disparity loop, aggregation and finalization.

## ASCII flow diagram
Left and Right images (YUV) -> YUV2RGB -> RGB2Gray -> Census  \
                                \                            \
                                 \---> ComputeWeights ------> AggregateCosts -> DisparitySelect -> MedianFilter -> Output
CostConstruction (per disparity) feeds AggregateCosts (per disparity).
(Edges denote data flow; cost construction runs inside disparity loop and produces per-disparity cost buffers consumed by aggregation.)

## Data dependency graph (text)
- Input: leftY, leftU, leftV, rightY, rightU, rightV
- yuv2rgb(leftY,U,V) -> leftRGB
- yuv2rgb(rightY,U,V) -> rightRGB
- rgb2Gray(leftRGB) -> grayL
- rgb2Gray(rightRGB) -> grayR
- census(grayL) -> cenL
- census(grayR) -> cenR
- computeWeights(grayL or RGB?) -> weight maps (horizontal/vertical) used by aggregation
- For each disparity d in [minDisparity..maxDisparity]:
  - costConstruction(grayL, grayR, cenL, cenR, d) -> cost_d (height*width floats)
  - aggregateCosts(cost_d, weights, ...) -> aggregatedContribution for d (or accumulate into aggregatedDisparity)
  - disparitySelect updates bestCost and result using aggregatedDisparity
- After disparity loop: medianFilter(result) -> final disparity map

Dependencies summary:
- Census depends only on grayscale images (no disparity).
- CostConstruction depends on grayscale + census + disparity value and produces a per-disparity cost buffer.
- AggregateCosts depends on per-disparity cost and weights (weights independent of disparity).
- DisparitySelect depends on aggregated costs across disparities (reduces into bestCost and disparity result).
- Median filter reads disparity result and writes final filtered disparity.

## Parallelization candidates
1. yuv2rgb: per-pixel independent conversion. Already uses OpenMP in code. Safe and high throughput.
   - Reason: reads only input Y/U/V and writes disjoint output pixel ranges.
2. rgb2Gray and census: per-pixel operations with no cross-pixel writes; can process rows/blocks in parallel.
   - Reason: each output pixel depends only on local input; census reads neighborhood but writes single pixel; use proper boundary handling or tile overlap.
3. costConstruction (per disparity): for a given disparity, per-pixel costs are independent — the per-disparity cost computation can be parallelized across pixels. Additionally, different disparity levels are independent, enabling coarse-grain parallelism across disparities if memory allows.
   - Two modes:
     - Parallelize pixels inside a disparity (lower memory).
     - Parallelize disparities (requires separate cost buffers or careful synchronization).
4. aggregateCosts: often more challenging but still parallelizable with careful handling:
   - If aggregation uses local neighborhood weights and produces aggregated cost per pixel from cost_d, each output pixel depends on a local window of cost_d and weights and thus can be computed in parallel (per-pixel) without races if output stored separately.
   - If aggregation is iterative or uses cumulative updates into shared bestCost buffers, use reduction patterns or per-thread temporary accumulation then merge.
5. disparitySelect: update of bestCost and result are per-pixel atomic/local operations for each pixel; within per-disparity loop this is easily parallelizable across pixels.
6. medianFilter: per-pixel output computed from local 3x3 window of input disparity; safe to parallelize when reading from input and writing to separate output buffer.

Parallelization caveats:
- Avoid race conditions on shared buffers (bestCost, aggregatedDisparity) — either use atomic updates, per-thread temporary buffers, or partition image so threads write disjoint slices.
- Cache locality: choose row-wise partitioning to preserve scan-line locality.
- Synchronization points: after computing census and weights, wait before per-disparity processing.

## Suggested improvements (non-breaking, incremental)
- Reuse single per-disparity cost buffer (allocate once) to reduce memory pressure.
- Free intermediate buffers (RGB) immediately after conversion to grayscale.
- Use OpenMP pragmas consistently for heavy per-pixel loops (rgb2Gray, census, costConstruction, aggregateCosts, disparitySelect, medianFilter).
- Consider blocking/tiling for aggregateCosts to improve cache reuse for neighborhood operations.
- Consider vectorizing inner arithmetic (use compiler-friendly loops, align data, or intrinsics if necessary).
- Provide compile-time options to choose between per-disparity streaming or full cost-volume allocation (memory vs. performance tradeoff).

## Testing strategy (practical steps)
1. Build and run small test images (e.g., 64×64) to inspect intermediate buffers.
2. Add temporary logging (printf) guarded by a macro (e.g., DEBUG_TRACE) to print:
   - Buffer sizes, strides, and a sample pixel value after each stage.
   - Loop iteration counts in the disparity loop.
3. Trace a single pixel path:
   - Choose test coordinates (x,y) and print values of inputs and after each stage (YUV, RGB, Gray, Census, per-disparity cost, aggregated cost, selected disparity).
4. Unit tests:
   - Small synthetic stereo pairs with known shift/disparity to validate pipeline correctness.
   - Compare outputs after converting code changes to a known-good baseline.
5. Performance profiling:
   - Profile with a representative resolution and disparity range to identify hotspots (costConstruction and aggregation expected to be top).
6. Regression:
   - Add a small end-to-end test that computes MD5 of final output and compare to baseline to detect regressions.

Recommended VS workflow:
- Use __Build__ to compile, run under __Debug__ or __Release__ and inspect the Output window.
- For performance testing use Release and measure with timers or OS profilers.

## Deliverables checklist for Issue 2.1
- [x] Algorithm flow diagram (textual/ASCII here)
- [x] Data dependency graph (section above)
- [x] Memory allocation map and lifetime notes
- [x] Complexity estimates (per stage)
- [x] At least 3 parallelization candidates with justification
- [x] Testing strategy and questions for maintainers
