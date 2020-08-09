// as seen on https://github.com/diegohaz/arc/wiki/Example-components#icon
import React from "react";
import styled from "styled-components";

export interface WrapperProps {
  width?: number;
  height?: number;
  color?: string;
  reverse?: boolean;
}

export type IconProps = WrapperProps & { icon: string; style?: object };

const fontSize = ({ width, height }: { width?: number; height?: number }) => {
  const size = width || height;
  return size ? `${size / 16}rem` : "1.25em";
};

const Wrapper = styled.span<WrapperProps>`
  display: inline-block;
  font-size: ${fontSize};
  color: ${({ color }) => (color ? `${color} !important` : "currentcolor")};
  width: 1em;
  height: 1em;
  margin: 0.1em;
  box-sizing: border-box;

  & > svg {
    width: 100%;
    height: 100%;
    fill: currentcolor;

    path,
    rect,
    ellipse {
      fill: currentcolor;
    }
  }
`;
export default function Icon({ icon, ...props }: IconProps) {
  const svg = require(`!raw-loader!../icons/${icon}.svg`);
  return (
    <Wrapper {...props} dangerouslySetInnerHTML={{ __html: svg.default }} />
  );
}
