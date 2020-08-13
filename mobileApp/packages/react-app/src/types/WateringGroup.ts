import { Device } from "./Device";

export interface WateringGroup {
  _id: string;
  displayName: string;
  ownedBy: string;
  lastPumped: string;
  moistureThreshold: number;
  minimalPumpInterval: number;
  devices: Device[];
}
