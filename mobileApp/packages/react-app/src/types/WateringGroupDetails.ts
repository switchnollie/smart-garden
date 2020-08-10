import { WateringGroup } from "./WateringGroup";
import { Device } from "./Device";

export interface WateringGroupDetails extends Omit<WateringGroup, "devices"> {
  devices: Device[];
}
