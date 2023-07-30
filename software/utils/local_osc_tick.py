#!/usr/bin/env python3

# Script to evaluate choices of tick size for generating
# local oscillator signals, in terms of centre frequency
# generation and phase error.


import numpy as np
import pandas as pd

def ticks_for_frequency(f, tick_period):
    '''
    Returns the number of ticks that synthesises the frequency
    f as closely as possible from the given tick_period.

    Note that this function does not account for dividing the
    resulting square wave into two exactly equal half cycles. 
    The low half-cycle may be one tick longer than the high
    half-cycle, if the number of ticks comes out odd. This does
    not affect the frequency.

    Returns the best number of ticks
    '''
    true_period = 1/f
    n_exact = true_period / tick_period
    # Let x be the "number" of ticks required to synthesise f exactly
    # (a real number). For n an integer, let n_exact = n + x.
    # The number of ticks producing the closest frequency to f
    # is either n or n + 1. By directly calculating the error, choose
    # n if x < n / (2n + 1). Unless n is really small, the distinction
    # does not matter too much.
    n = np.floor(n_exact)
    x = n_exact - n
    if x < n / (2*n + 1):
        return int(n)
    else:
        return int(n + 1)

def frequency_error(f, n, tick_period):
    '''
    Compute the relative error in synthesising the frequency f using
    n ticks of the tick_period specified
    '''
    return np.abs((f - 1/(n*tick_period)) / f)

    
def phase_error(n, tick_period):
    '''
    Calculate the phase error between the local oscillator
    in-phase and quadrature signals while synthesising the
    frequency f using the tick_period specified. 

    For two square pulses synthesised from n ticks, assuming
    the ticks are synchronised between the signals, one
    signal must be offset from the other by n/4 (to make 90 deg)
    The phase error is the relative difference between the
    closest integer m to n/4 and n/4 itself
    '''
    m_exact = n/4;
    m = np.round(n/4)
    return np.abs(m - m_exact)/m_exact


if __name__ == "__main__":
    # Required centre frequencies
    f_list = np.arange(535e3, 1606e3, 1e3) # 535 kHz to 1605 kHz,

    tick_period = 1/80e6 # 80 MHz (APB clock in esp32 c3)

    n_list = [ticks_for_frequency(f, tick_period) for f in f_list]
    f_err = [frequency_error(f, n, tick_period) for (f, n) in zip(f_list, n_list)]
    p_err = [phase_error(n, tick_period) for n in n_list]

    df = pd.DataFrame({
        "f": f_list,
        "n": n_list,
        "f_err": f_err,
        "p_err": p_err
    })

    print(df)
    
    print(df.describe())
