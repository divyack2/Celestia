#include <cmath>
#include <memory>
#include <sstream>

#include <celephem/orbit.h>

#include <doctest.h>

using celestia::ephem::HyperbolicOrbit;

TEST_SUITE_BEGIN("HyperbolicOrbit::eccentricAnomaly (Newton–Raphson)");

/// Helper to build a HyperbolicOrbit with e>1, a dummy a<0 and period
static HyperbolicOrbit makeHyp(double e)
{
    celestia::astro::KeplerElements el{};
    el.eccentricity      = e;
    el.semimajorAxis     = -5000.0;  // hyperbola needs a<0
    el.longAscendingNode = 0.0;
    el.inclination       = 0.0;
    el.argPericenter     = 0.0;
    el.meanAnomaly       = 0.0;      // will override in tests
    el.period            = 1000.0;   // dummy non-zero
    return HyperbolicOrbit(el, /*epoch=*/0.0);
}

TEST_CASE("M == 0 → E == 0")
{
    auto hyp = makeHyp(1.3);
    CHECK(hyp.eccentricAnomalyHelper(0.0) == doctest::Approx(0.0));
}

TEST_CASE("Tiny |M| < 1e-6 uses linear approx E ≈ M/(e-1)")
{
    const double e = 2.7;
    auto hyp = makeHyp(e);

    double smallMs[] = { 5e-7, -2e-7, 9.999e-7 };
    for (double M : smallMs)
    {
        double E_code = hyp.eccentricAnomalyHelper(M);
        double E_lin  = M / (e - 1.0);
        CHECK(E_code == doctest::Approx(E_lin).epsilon(1e-12));
    }
}

TEST_CASE("General round-trip consistency over a grid of F")
{
    constexpr double EPS = 1e-9; // chose this arbitrarily
    const double e = 1.5;
    auto hyp = makeHyp(e);

    // Sweep true anomaly F from –4 to +4 in steps of 0.5
    for (double F_true = -4.0; F_true <= 4.0; F_true += 0.5)
    {
        // Mean anomaly from Kepler's hyperbolic equation
        double M = e * std::sinh(F_true) - F_true;

        // Solve back to F_code
        double F_code = hyp.eccentricAnomalyHelper(M);

        // Residual f(F) = e sinh(F) – F – M should be ≈ 0
        double res = e * std::sinh(F_code) - F_code - M;
        CHECK(res == doctest::Approx(0.0).epsilon(EPS));

        // And F_code close to original F_true
        CHECK(F_code == doctest::Approx(F_true).epsilon(EPS));
    }
}

TEST_SUITE_END();
