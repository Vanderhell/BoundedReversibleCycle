#include "brc.h"

#include <stddef.h>

#define BRC_DELTA_TAG UINT32_C(0x42524431)
#define BRC_Q_TAG UINT32_C(0x42525131)

static uint32_t brc_next32(uint32_t *state)
{
    uint32_t z;
    *state += UINT32_C(0x9E3779B9);
    z = *state;
    z = (z ^ (z >> 16)) * UINT32_C(0x85EBCA6B);
    z = (z ^ (z >> 13)) * UINT32_C(0xC2B2AE35);
    return z ^ (z >> 16);
}

static uint32_t brc_mix32(uint32_t x)
{
    x ^= x >> 16;
    x *= UINT32_C(0x85EBCA6B);
    x ^= x >> 13;
    x *= UINT32_C(0xC2B2AE35);
    x ^= x >> 16;
    return x;
}

static uint32_t brc_partner32(uint32_t x, uint32_t pivot, uint32_t domain)
{
    return pivot >= x ? pivot - x : domain - (x - pivot);
}

static uint16_t brc_partner16(uint16_t x, uint16_t pivot, uint16_t domain)
{
    return pivot >= x ? (uint16_t)(pivot - x) : (uint16_t)(domain - (uint16_t)(x - pivot));
}

static uint32_t brc_round32(uint32_t x, const brc32_round_t *r, uint32_t n)
{
    uint32_t p = brc_partner32(x, r->pivot, n);
    uint32_t c = x < p ? x : p;
    return (brc_mix32(c ^ r->salt) & 1u) != 0u ? p : x;
}

static uint16_t brc_round16(uint16_t x, const brc16_round_t *r, uint16_t n)
{
    uint16_t p = brc_partner16(x, r->pivot, n);
    uint16_t c = x < p ? x : p;
    return (brc_mix32((uint32_t)c ^ (uint32_t)r->salt) & 1u) != 0u ? p : x;
}

static uint32_t brc_gcd32(uint32_t a, uint32_t b)
{
    while (b != 0u) { uint32_t r = a % b; a = b; b = r; }
    return a;
}

static uint32_t brc_choose_delta(uint32_t n, uint32_t *state)
{
    uint32_t d, attempt;
    if (n == 1u) return 0u;
    d = (brc_next32(state) % (n - 1u)) + 1u;
    for (attempt = 0u; attempt < 32u; ++attempt) {
        if (brc_gcd32(d, n) == 1u) return d;
        d = d == n - 1u ? 1u : d + 1u;
    }
    return 1u; /* Always coprime; fixed upper bound on initialization work. */
}

bool brc32_init_with_delta_v1(brc32_ctx_t *ctx, uint32_t domain, uint32_t delta, uint32_t seed)
{
    uint32_t state, i;
    if (ctx == NULL || domain == 0u) return false;
    if (domain == 1u) { if (delta != 0u) return false; }
    else if (delta == 0u || delta >= domain) return false;
    state = seed ^ domain ^ BRC_Q_TAG;
    ctx->domain = domain;
    ctx->delta = delta;
    ctx->threshold = domain - delta;
    for (i = 0u; i < BRC32_V1_Q_ROUNDS; ++i) {
        ctx->q[i].pivot = brc_next32(&state) % domain;
        ctx->q[i].salt = brc_next32(&state);
    }
    return true;
}

bool brc16_init_with_delta_v1(brc16_ctx_t *ctx, uint16_t domain, uint16_t delta, uint32_t seed)
{
    uint32_t state, i;
    if (ctx == NULL || domain == 0u) return false;
    if (domain == 1u) { if (delta != 0u) return false; }
    else if (delta == 0u || delta >= domain) return false;
    state = seed ^ (uint32_t)domain ^ BRC_Q_TAG;
    ctx->domain = domain;
    ctx->delta = delta;
    ctx->threshold = (uint16_t)(domain - delta);
    for (i = 0u; i < BRC16_V1_Q_ROUNDS; ++i) {
        ctx->q[i].pivot = (uint16_t)(brc_next32(&state) % (uint32_t)domain);
        ctx->q[i].salt = (uint16_t)brc_next32(&state);
    }
    return true;
}

bool brc32_init_v1(brc32_ctx_t *ctx, uint32_t domain, uint32_t seed)
{
    uint32_t state, delta;
    if (ctx == NULL || domain == 0u) return false;
    state = seed ^ domain ^ BRC_DELTA_TAG;
    delta = brc_choose_delta(domain, &state);
    return brc32_init_with_delta_v1(ctx, domain, delta, seed);
}

bool brc16_init_v1(brc16_ctx_t *ctx, uint16_t domain, uint32_t seed)
{
    uint32_t state, delta;
    if (ctx == NULL || domain == 0u) return false;
    state = seed ^ (uint32_t)domain ^ BRC_DELTA_TAG;
    delta = brc_choose_delta((uint32_t)domain, &state);
    return brc16_init_with_delta_v1(ctx, domain, (uint16_t)delta, seed);
}

static uint32_t brc_q32(uint32_t x, const brc32_ctx_t *ctx)
{
    uint32_t i;
    for (i = 0u; i < BRC32_V1_Q_ROUNDS; ++i) x = brc_round32(x, &ctx->q[i], ctx->domain);
    return x;
}

static uint32_t brc_q32_inverse(uint32_t x, const brc32_ctx_t *ctx)
{
    uint32_t i = BRC32_V1_Q_ROUNDS;
    while (i != 0u) { --i; x = brc_round32(x, &ctx->q[i], ctx->domain); }
    return x;
}

static uint16_t brc_q16(uint16_t x, const brc16_ctx_t *ctx)
{
    uint32_t i;
    for (i = 0u; i < BRC16_V1_Q_ROUNDS; ++i) x = brc_round16(x, &ctx->q[i], ctx->domain);
    return x;
}

static uint16_t brc_q16_inverse(uint16_t x, const brc16_ctx_t *ctx)
{
    uint32_t i = BRC16_V1_Q_ROUNDS;
    while (i != 0u) { --i; x = brc_round16(x, &ctx->q[i], ctx->domain); }
    return x;
}

uint32_t brc32_forward_v1(uint32_t x, const brc32_ctx_t *ctx)
{
    uint32_t q;
    if (ctx == NULL) return 0u;
    if (ctx->domain == 0u || x >= ctx->domain) return x;
    q = brc_q32(x, ctx);
    q = q >= ctx->threshold ? q - ctx->threshold : q + ctx->delta;
    return brc_q32_inverse(q, ctx);
}

uint32_t brc32_inverse_v1(uint32_t x, const brc32_ctx_t *ctx)
{
    uint32_t q;
    if (ctx == NULL) return 0u;
    if (ctx->domain == 0u || x >= ctx->domain) return x;
    q = brc_q32(x, ctx);
    q = q >= ctx->delta ? q - ctx->delta : q + ctx->threshold;
    return brc_q32_inverse(q, ctx);
}

uint16_t brc16_forward_v1(uint16_t x, const brc16_ctx_t *ctx)
{
    uint16_t q;
    if (ctx == NULL) return 0u;
    if (ctx->domain == 0u || x >= ctx->domain) return x;
    q = brc_q16(x, ctx);
    q = q >= ctx->threshold ? (uint16_t)(q - ctx->threshold) : (uint16_t)(q + ctx->delta);
    return brc_q16_inverse(q, ctx);
}

uint16_t brc16_inverse_v1(uint16_t x, const brc16_ctx_t *ctx)
{
    uint16_t q;
    if (ctx == NULL) return 0u;
    if (ctx->domain == 0u || x >= ctx->domain) return x;
    q = brc_q16(x, ctx);
    q = q >= ctx->delta ? (uint16_t)(q - ctx->delta) : (uint16_t)(q + ctx->threshold);
    return brc_q16_inverse(q, ctx);
}
