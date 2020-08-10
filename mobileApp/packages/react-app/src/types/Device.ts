export interface Device {
  _id: string;
  logBuckets: string[];
  displayName: string;
  type: "moisture" | "waterlevel" | "pump";
  ownedBy: string;
}
