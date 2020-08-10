import React from "react";
import styled, { css } from "styled-components";
import { useCountUp } from "react-countup";

interface LabeledStatProps {
  primary?: boolean;
  secondary?: boolean;
  value: number | string;
  label: string;
}

type LabeledStatHocProps = LabeledStatProps & { countUp?: boolean };

const getColor = ({
  primary,
  secondary,
}: Pick<LabeledStatProps, "primary" | "secondary">) => {
  if (primary) {
    return "var(--ion-color-primary)";
  } else if (secondary) {
    return "var(--ion-color-danger)";
  }
  return "var(--ion-text-color)";
};

const PercentageText = styled.h2<{ primary?: boolean; secondary?: boolean }>`
  ${({ primary, secondary, theme }) => css`
    font-size: 2.25rem !important;
    font-weight: bold !important;
    color: ${getColor({ primary, secondary })};
    line-height: 1;
    margin-top: 2px;
    margin-bottom: 2px;
  `}
`;

const CountLabel = styled.h4`
  ${({ theme }) => css`
    font-size: 0.875rem !important;
    color: ${theme.colors.font.fontPrimary};
    font-weight: bold !important;
    margin-top: 2px;
    margin-bottom: 2px;
  `}
`;

const LabeledStat = ({
  primary,
  secondary,
  value,
  label,
}: LabeledStatProps) => (
  <div>
    <PercentageText primary={primary} secondary={secondary}>
      {value}
    </PercentageText>
    <CountLabel>{label}</CountLabel>
  </div>
);

function CountUpStat({ value, ...props }: LabeledStatProps) {
  const { countUp: counterValue } = useCountUp({
    start: 0,
    duration: 0.7,
    decimals: 0,
    delay: 0.5,
    suffix: " %",
    end: value as number,
  });
  return <LabeledStat {...props} value={counterValue} />;
}

export default ({ countUp, value, ...props }: LabeledStatHocProps) =>
  countUp && !isNaN(value as any) ? (
    <CountUpStat value={value} {...props} />
  ) : (
    <LabeledStat value={value} {...props} />
  );
