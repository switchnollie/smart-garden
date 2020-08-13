import useSWR, { mutate } from "swr";
import { WateringGroup } from "../types/WateringGroup";
import { getTenBitAnalog } from "../utils/relativeAnalogValues";
import produce from "immer";

export default function useWateringGroups() {
  const uri = `/api/wateringgroup`;
  const { data, error } = useSWR<WateringGroup[]>(uri);

  const updateGroupParam = async (
    wateringGroup?: WateringGroup,
    updatedKey?: keyof WateringGroup | null,
    updatedValue?: any
  ) => {
    if (data && wateringGroup && updatedValue && updatedKey) {
      const selectedGroupIndex = data.findIndex(
        (group) => group._id === wateringGroup._id
      );
      // Only update if changed
      console.log({ updatedValue, currentValue: wateringGroup[updatedKey] });
      if (updatedValue !== wateringGroup[updatedKey]) {
        const updatedData = produce(data, (draft) => {
          (draft[selectedGroupIndex] as { [key: string]: any })[
            updatedKey
          ] = updatedValue;
        });
        // Update local cache immediately
        mutate(uri, updatedData, false);
        // Send update API Request
        await fetch(`${uri}/${wateringGroup._id}`, {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
          },
          body: JSON.stringify({ [updatedKey]: updatedValue }),
        });
      }
    }
  };

  const updateMoistureThreshold = async (
    newMoistureThreshold: number | null,
    wateringGroup?: WateringGroup
  ) => {
    await updateGroupParam(
      wateringGroup,
      "moistureThreshold",
      newMoistureThreshold ? getTenBitAnalog(newMoistureThreshold) : null
    );
  };

  const updateMinimalInterval = async (
    newInterval: number | null,
    wateringGroup?: WateringGroup
  ) => {
    await updateGroupParam(wateringGroup, "minimalPumpInterval", newInterval);
  };

  return {
    groups: data,
    isLoading: !error && !data,
    isError: error,
    updateMoistureThreshold,
    updateMinimalInterval,
  };
}
