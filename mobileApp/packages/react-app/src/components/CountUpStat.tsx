import React, { useState } from "react";
import styled, { css, keyframes } from "styled-components";
import { useCountUp } from "react-countup";

interface CountUpStatProps {
  primary?: boolean;
  secondary?: boolean;
  value: number;
  label: string;
}

const PercentageText = styled.h2<{ primary?: boolean; secondary?: boolean }>`
  ${({ secondary, theme }) => css`
    font-size: 2.25rem !important;
    font-weight: bold !important;
    color: ${secondary ? theme.colors.accentRed : theme.colors.accentBlue};
    line-height: 1;
  `}
`;

const CountLabel = styled.h4`
  ${({ theme }) => css`
    font-size: 0.875rem !important;
    color: ${theme.colors.font.fontPrimary};
    font-weight: bold !important;
  `}
`;

export default function CountUpStat({
  primary,
  secondary,
  value,
  label,
}: CountUpStatProps) {
  const { countUp: counterValue } = useCountUp({
    start: 0,
    duration: 0.7,
    decimals: 0,
    delay: 0.5,
    suffix: " %",
    end: value,
  });
  return (
    <div>
      <PercentageText primary={primary} secondary={secondary}>
        {counterValue}
      </PercentageText>
      <CountLabel>{label}</CountLabel>
    </div>
  );
}
