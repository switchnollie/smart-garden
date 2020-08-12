import useSWR from "swr";
import { WateringGroup } from "../types/WateringGroup";

export default function useWateringGroups() {
  const { data, error } = useSWR<WateringGroup[]>(`/api/wateringgroup`);

  return {
    groups: data,
    isLoading: !error && !data,
    isError: error,
  };
}
