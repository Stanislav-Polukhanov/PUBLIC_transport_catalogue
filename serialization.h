#pragma once

#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

TransportCataloguePB::TransportCatalogue SerializeCatalogue(const TransportCatalogue& catalogue);
TransportCataloguePB::RenderSettings SerializeRenderSettings(const RenderSettings& render_settings);
TransportCataloguePB::RoutingSettings SerializeRoutingSettings(const RoutingSettings& routing_settings);

void SerializeAll(
	const TransportCatalogue& catalogue_pb, 
	const RenderSettings& render_settings, 
	const RoutingSettings& routing_settings,
	std::ostream& out);

void DeserializeCatalogueInPlace(const TransportCataloguePB::TransportCatalogue& catalogue_database_pb, TransportCatalogue& catalogue);
void DeserializeRenderSettingsInPlace(const TransportCataloguePB::RenderSettings& render_settings_pb, RenderSettings& render_settings);
void DeserializeRoutingSettingsInPlace(const TransportCataloguePB::RoutingSettings& routing_settings_pb, RoutingSettings& routing_settings);
void DeserializeAllInPlace(std::istream& in, TransportCatalogue& catalogue, RenderSettings& render_settings, RoutingSettings& routing_settings);