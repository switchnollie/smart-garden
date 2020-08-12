export function getTenBitPercentage(analogVal: number) {
  return Math.round((analogVal / 1023) * 100);
}

export function getTenBitAnalog(percentage: number) {
  return Math.round((percentage / 100) * 1023);
}
