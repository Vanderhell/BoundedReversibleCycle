#ifndef BRC_H
#define BRC_H

#include <stdbool.h>
#include <stdint.h>

/* BRC-V1 conjugates a coprime domain rotation by two BRP-style rounds. */
#define BRC_ALGORITHM_VERSION 1u
#define BRC16_V1_Q_ROUNDS 2u
#define BRC32_V1_Q_ROUNDS 2u

typedef struct { uint32_t pivot; uint32_t salt; } brc32_round_t;
typedef struct {
    uint32_t domain;
    uint32_t delta;
    uint32_t threshold;
    brc32_round_t q[BRC32_V1_Q_ROUNDS];
} brc32_ctx_t;

typedef struct { uint16_t pivot; uint16_t salt; } brc16_round_t;
typedef struct {
    uint16_t domain;
    uint16_t delta;
    uint16_t threshold;
    brc16_round_t q[BRC16_V1_Q_ROUNDS];
} brc16_ctx_t;

/* Seeded initialization deterministically selects a coprime delta. */
bool brc32_init_v1(brc32_ctx_t *ctx, uint32_t domain, uint32_t seed);
bool brc16_init_v1(brc16_ctx_t *ctx, uint16_t domain, uint32_t seed);
/* Explicit step initialization. Any 1 <= delta < domain is accepted; coprime
 * delta gives full-cycle mode, other deltas retain gcd(domain,delta) cycles. */
bool brc32_init_with_delta_v1(brc32_ctx_t *ctx, uint32_t domain, uint32_t delta, uint32_t seed);
bool brc16_init_with_delta_v1(brc16_ctx_t *ctx, uint16_t domain, uint16_t delta, uint32_t seed);

/* Valid calls require an initialized context and x < domain. Invalid x is
 * returned unchanged; NULL context returns zero, matching the BRP convention. */
uint32_t brc32_forward_v1(uint32_t x, const brc32_ctx_t *ctx);
uint32_t brc32_inverse_v1(uint32_t x, const brc32_ctx_t *ctx);
uint16_t brc16_forward_v1(uint16_t x, const brc16_ctx_t *ctx);
uint16_t brc16_inverse_v1(uint16_t x, const brc16_ctx_t *ctx);

#endif
