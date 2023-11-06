import pandas as pd
import numpy as np


def roll_from_csv(filename):
    df = pd.read_csv(filename)
    df = df[["accZ"]]

    arr = np.array([window.to_numpy() for window in df.rolling(20, min_periods=20)][19:])

    return arr


def filter_mad(arr):
    vol = np.sum(np.abs(arr), axis=(1,2))
    median = np.median(vol)
    mad = np.median(np.abs(vol - median))

    poi = vol > 3*mad + median
    print("left after filtering:", np.sum(poi) / vol.shape[0])

    return arr[poi, :, :]


def export_dataset(no_taps, soft_taps, hard_taps, balance=True):
    yes_taps = np.concatenate((soft_taps, hard_taps))
    if balance:
        no_no_taps = min(no_taps.shape[0], yes_taps.shape[0])
    else:
        no_no_taps = -1

    x = np.concatenate((
        no_taps.reshape((no_taps.shape[0], -1))[:no_no_taps],
        yes_taps.reshape((yes_taps.shape[0], -1))[:no_no_taps]
    ))
    y = np.concatenate((
        np.repeat(0, no_taps.shape[0])[:no_no_taps],
        np.repeat(1, yes_taps.shape[0])[:no_no_taps]
    ))
    y = np.eye(2)[y]

    print("X shape", x.shape)
    print("Y shape", y.shape)

    assert(x.shape[0] == y.shape[0])

    shuffle = np.arange(x.shape[0])
    np.random.shuffle(shuffle)
    x = x[shuffle]
    y = y[shuffle]

    np.save("data/input.npy", x)
    np.save("data/output.npy", y)


def main():
    ht = roll_from_csv("data/hardtap.csv")
    ht = filter_mad(ht)

    ht2 = roll_from_csv("data/hardtap2.csv")
    ht2 = filter_mad(ht2)

    st = roll_from_csv("data/softtap.csv")
    st = filter_mad(st)

    st2 = roll_from_csv("data/softtap2.csv")
    st2 = filter_mad(st2)

    nt = roll_from_csv("data/notap.csv")

    export_dataset(nt, np.concatenate((st, st2)), np.concatenate((st, st2)))


if __name__ == "__main__":
    main()
