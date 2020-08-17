export interface Device {
  _id: string;
  logBuckets: string[];
  lastValue: any;
  displayName: string;
  type: "moisture" | "waterlevel" | "pump";
  ownedBy: string;
}
