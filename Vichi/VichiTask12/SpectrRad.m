function spectr = SpectrRad(A)

arr=(eig(A));
spectr=max(abs(arr));

end

