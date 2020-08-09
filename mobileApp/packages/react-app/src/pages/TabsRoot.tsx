import React from "react";
import { Route, Redirect } from "react-router-dom";
import {
  IonPage,
  IonTabs,
  IonRouterOutlet,
  IonTabBar,
  IonTabButton,
  IonIcon,
  IonLabel,
} from "@ionic/react";
import { PlantsPage, InsightsPage, ProfilePage, PlantDetailsPage } from "./";
import Icon from "../components/Icon";

export default function TabsRoot() {
  return (
    <IonPage>
      <IonTabs>
        <IonRouterOutlet>
          <Route exact path="/" render={() => <Redirect to="/plants" />} />
          <Route path="/:tab(plants)" exact component={PlantsPage} />
          <Route path="/:tab(plants)/:groupName" component={PlantDetailsPage} />
          <Route path="/:tab(insights)" exact component={InsightsPage} />
          <Route path="/:tab(profile)" exact component={ProfilePage} />
        </IonRouterOutlet>
        <IonTabBar slot="bottom" mode="ios">
          <IonTabButton tab="plants" href="/plants">
            <Icon height={20} icon="seed" />
            <IonLabel>Plants</IonLabel>
          </IonTabButton>
          <IonTabButton tab="insights" href="/insights">
            <Icon height={20} icon="insights" />
            <IonLabel>Insights</IonLabel>
          </IonTabButton>
          <IonTabButton tab="profile" href="/profile">
            <Icon height={20} icon="profile" />
            <IonLabel>Profile</IonLabel>
          </IonTabButton>
        </IonTabBar>
      </IonTabs>
    </IonPage>
  );
}
