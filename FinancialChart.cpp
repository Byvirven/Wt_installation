#include "FinancialChart.hpp"

namespace Financial {

//Chart::Chart(const std::shared_ptr< Wt::WAbstractItemModel > & model)
Chart::Chart(HDF5Reader * reader, int timezoneOffset)
    : Wt::Chart::WCartesianChart()
{
	monitoring_ = false ;
	filereader_ = reader ;
	timezoneOffset_ = timezoneOffset ;
	
	createModel();
	
	setPreferredMethod(Wt::RenderMethod::HtmlCanvas);
	setType(Wt::Chart::ChartType::Scatter);
	setModel(model_);
	setXSeriesColumn(0);
	setZoomEnabled(true);
	setPanEnabled(true);
	setCrosshairEnabled(true);
	setRubberBandEffectEnabled(true);
	setLegendEnabled(false);

	// Arrière-plan sombre
	setBackground(Wt::WColor(30, 30, 30));   // Fond sombre

	// Insérer les données dans le modèle
	updateModel();
	
	// Initialiser les valeurs min/max pour les axes
	updateAxisRanges();

	// configurer les séries
	setupSeries();
	
	// configurer les axes
	setupAxes();

	// configurer la mire
	setupCrosshair();

	// Maximiser la taille automatiquement en fonction du widget parent
	setMaximumSize(Wt::WLength::Auto, Wt::WLength::Auto);
	
	// activer la surveillance pour mise à jour
	//startMonitoring();
	// Configurer le timer pour mettre à jour toutes les minutes
	timer_ = addChild(std::make_unique<Wt::WTimer>());
	timer_->setInterval(std::chrono::minutes(1));
	timer_->timeout().connect(this, &Chart::onFileModified);
	timer_->start();
}

Chart::~Chart() {
	stopMonitoring();
}

void Chart::updateAxisRanges() {	
	// définir les bornes minimales et maximales
	double minX = minTime_;
	double maxX = maxTime_;
	double maxY = maxPrice_;
	double minY = minPrice_;
	// Définir le zoom maximale (x2056)
	axis(Wt::Chart::Axis::X).setMinimumZoomRange((maxX - minX) / 2056);
	
	// définir les bornes minimales et maximales de l'axe X
	axis(Wt::Chart::Axis::X).setRange(minX, maxX);
	
	// Identifier la taille des parties entière et décimale et formater dynamiquement la dimension et le label Y
	int decimal_part = std::ceil(-std::log10(minY)) ;
	decimal_part = static_cast<int>(std::min((decimal_part > 0 ? decimal_part+3 : 2), 8)) ;
	int integer_part = std::max(static_cast<int>(std::log10(maxY)) + 1, 2) ;
	setPlotAreaPadding(12*(decimal_part+integer_part+1), Wt::Side::Left);
	char format[10];
	snprintf(format, sizeof(format), "%%.%df", decimal_part);
	axis(Wt::Chart::Axis::Y).setLabelFormat(Wt::WString::fromUTF8(format));
	axis(Wt::Chart::Axis::Y).setAutoLimits(Wt::Chart::AxisValue::Minimum | Wt::Chart::AxisValue::Maximum);
	axis(Wt::Chart::Axis::Y).setMaximum(maxY*1.0000001);
	axis(Wt::Chart::Axis::Y).setMinimum(minY*0.9999999);
	axis(Wt::Chart::Axis::X).setMaximum(maxX);
	axis(Wt::Chart::Axis::X).setMinimum(minX);
}

void Chart::setupSeries() {
	// Séries de données pour Open, High, Low, Close
	auto openSeries = std::make_unique<Wt::Chart::WDataSeries>(1, Wt::Chart::SeriesType::Line);
	auto highSeries = std::make_unique<Wt::Chart::WDataSeries>(2, Wt::Chart::SeriesType::Line);
	auto lowSeries = std::make_unique<Wt::Chart::WDataSeries>(3, Wt::Chart::SeriesType::Line);
	auto closeSeries = std::make_unique<Wt::Chart::WDataSeries>(4, Wt::Chart::SeriesType::Line);

	// Utilisation de couleurs sobres pour chaque série
	openSeries->setPen(Wt::WPen(Wt::WColor(200, 200, 200)));  // Gris clair
	highSeries->setPen(Wt::WPen(Wt::WColor(255, 165, 0)));    // Orange
	lowSeries->setPen(Wt::WPen(Wt::WColor(0, 191, 255)));     // Bleu ciel
	closeSeries->setPen(Wt::WPen(Wt::WColor(255, 69, 0)));    // Rouge foncé

	// Couleurs des étiquettes des axes
	openSeries->setLabelColor(Wt::WColor(255, 255, 255)); // Couleur des labels en blanc pour plus de visibilité
	highSeries->setLabelColor(Wt::WColor(255, 255, 255)); // Couleur des labels en blanc pour plus de visibilité
	lowSeries->setLabelColor(Wt::WColor(255, 255, 255)); // Couleur des labels en blanc pour plus de visibilité
	closeSeries->setLabelColor(Wt::WColor(255, 255, 255)); // Couleur des labels en blanc pour plus de visibilité

	openSeries->setHidden(true);
	highSeries->setHidden(true);
	lowSeries->setHidden(true);
	closeSeries->setHidden(true);

	addSeries(std::move(openSeries));
	addSeries(std::move(highSeries));
	addSeries(std::move(lowSeries));
	addSeries(std::move(closeSeries));
}

void Chart::setupAxes() {
	// Ajustements des axes
	axis(Wt::Chart::Axis::X).setScale(Wt::Chart::AxisScale::DateTime);
	axis(Wt::Chart::Axis::X).setLabelFormat("yyyy-MM-dd hh:mm:ss");
	axis(Wt::Chart::Axis::X).setTitle("Date");

	// Couleurs des axes et des étiquettes
	axis(Wt::Chart::Axis::X).setPen(Wt::WPen(Wt::WColor(200, 200, 200))); // Axe X gris clair
	axis(Wt::Chart::Axis::Y).setPen(Wt::WPen(Wt::WColor(200, 200, 200))); // Axe Y gris clair

	// Grille en couleur sombre pour faciliter la lecture
	axis(Wt::Chart::Axis::X).setGridLinesEnabled(true);
	axis(Wt::Chart::Axis::X).setGridLinesPen(Wt::WPen(Wt::WColor(50, 50, 50)));  // Grille foncée pour l'axe X
	axis(Wt::Chart::Axis::Y).setGridLinesEnabled(true);
	axis(Wt::Chart::Axis::Y).setGridLinesPen(Wt::WPen(Wt::WColor(50, 50, 50)));  // Grille foncée pour l'axe Y

	// Police pour les étiquettes de l'axe X
	Wt::WFont xAxisFont;
	xAxisFont.setSize(Wt::WLength(8));
	axis(Wt::Chart::Axis::X).setLabelFont(xAxisFont);

	// Police pour les étiquettes de l'axe Y
	Wt::WFont yAxisFont;
	yAxisFont.setSize (Wt::WLength (8, Wt::LengthUnit::Pixel));
	axis(Wt::Chart::Axis::Y).setLabelFont(yAxisFont);

	// Définir la couleur des étiquettes des axes en blanc
	axis(Wt::Chart::Axis::X).setTextPen(Wt::WPen(Wt::WColor(255, 255, 255)));
	axis(Wt::Chart::Axis::Y).setTextPen(Wt::WPen(Wt::WColor(255, 255, 255)));
}

void Chart::setupCrosshair() {
	// Couleurs du crosshair pour un contraste élevé
	setCrosshairColor(Wt::WColor(255, 255, 255)); // Crosshair en blanc pour un bon contraste
	setCrosshairXAxis (0); // Crosshair en blanc pour un bon contraste
	setCrosshairYAxis(0); // Crosshair en blanc pour un bon contraste
}

// créer le modèle pour les bougies
void Chart::createModel() {

	//auto data = filereader_->loadHDF5Data();
	//Wt::log("info") << "Load data : " << data.size();
	//model_ = std::make_shared<Wt::WStandardItemModel>(data.size(), 7);
	model_ = std::make_shared<Wt::WStandardItemModel>(0, 7);
	model_->setHeaderData(0, Wt::Orientation::Horizontal, "Date");
	model_->setHeaderData(1, Wt::Orientation::Horizontal, "Open");
	model_->setHeaderData(2, Wt::Orientation::Horizontal, "High");
	model_->setHeaderData(3, Wt::Orientation::Horizontal, "Low");
	model_->setHeaderData(4, Wt::Orientation::Horizontal, "Close");
	model_->setHeaderData(5, Wt::Orientation::Horizontal, "Volume");
	model_->setHeaderData(6, Wt::Orientation::Horizontal, "Trades");
}

// créer le modèle pour les bougies
void Chart::updateModel() {
	auto data = filereader_->loadHDF5Data();
	size_t start_candle = model_->rowCount();
	
	if (start_candle == 0) { minTime_ = ((data[0].timestamp/1000) + (timezoneOffset_ * 60)) - 60; }
	maxTime_ = ((data[data.size()-1].timestamp/1000) + (timezoneOffset_ * 60)) + 60 ;
	
	// Remplir le modèle avec les données après avoir recréé
	for (size_t i = start_candle; i < data.size(); ++i) {
		model_->insertRow(model_->rowCount());
		model_->setData(i, 0, Wt::WDateTime(std::chrono::system_clock::time_point(std::chrono::seconds((data[i].timestamp/1000) + (timezoneOffset_ * 60))))) ;
		model_->setData(i, 1, data[i].open);
		model_->setData(i, 2, data[i].high);
		model_->setData(i, 3, data[i].low);
		model_->setData(i, 4, data[i].close);
		model_->setData(i, 5, data[i].volume);
		model_->setData(i, 6, data[i].trades);
		if (i==0 || data[i].high > maxPrice_) {
			maxPrice_ = data[i].high;
		} 
		if (i==0 || data[i].low < minPrice_) {
			minPrice_ = data[i].low;
		}
	}
}

void Chart::setSeriesConfigurations(const std::vector<Financial::SeriesConfig>& configs)
{
	seriesConfigs_ = configs;
}

void Chart::renderOther(Wt::WPainter &painter) const {
	Wt::Chart::WCartesianChart::renderOther(painter);

	if (!model()) { return; }

	painter.save();
	painter.setClipping(true);

	// get the the current zoom range transform
	Wt::WTransform zoomTransform = zoomRangeTransform();

	// sélectionner les séries
	for (const auto& config : seriesConfigs_) {
		switch (config.type) {
			case Financial::ChartType::Candlestick : {
				// Distance en 2 bougies
				double candleWidth = (Wt::asNumber(model()->data(1, 0)) - Wt::asNumber(model()->data(0, 0))) * 0.9;

				for (int row = 0; row < model()->rowCount(); ++row) {
					double timestamp = Wt::asNumber(model()->data(row, 0)) ; // timestamp
					double open = Wt::asNumber(model()->data(row, config.seriesIds[0])) ; // open price
					double high = Wt::asNumber(model()->data(row, config.seriesIds[1])) ; // high price
					double low = Wt::asNumber(model()->data(row, config.seriesIds[2])) ; // low price
					double close = Wt::asNumber(model()->data(row, config.seriesIds[3])) ; // close price

					Wt::WColor color = (close > open) ? Wt::WColor(0, 255, 0) : Wt::WColor(255, 0, 0);
					painter.setPen(Wt::WPen(color));
					painter.setBrush(Wt::WBrush(color));

					painter.drawPath
					(
						zoomTransform.map
						(
							drawCandle ( timestamp, open, high, low, close, candleWidth )
						)
					);
				}
				break;
			}
			case Financial::ChartType::Surface : {
				// Créer les chemins pour les deux séries
				int rowCount = model()->rowCount() ;
				int totalPoints = 2 * rowCount ;
				std::vector<Wt::WPointF> points( totalPoints + 1);
				
				for (int row = 0; row < rowCount; ++row) {
					double timestamp = Wt::asNumber(model()->data(row, 0));
					
					points[row] = mapToDeviceWithoutTransform
					(
						timestamp, 
						Wt::asNumber(model()->data(row, config.seriesIds[0]))
					) ;
					points[totalPoints - row - 1] = mapToDeviceWithoutTransform
					(
						timestamp, 
						Wt::asNumber(model()->data(row, config.seriesIds[1]))
					) ;
				}
				// ajouter le point de fermeture
				points[ totalPoints ] = points[0] ;
				
				// Créer un chemin pour la zone entre les deux lignes
				Wt::WPainterPath fillPath;
				fillPath.addPolygon(points); // composer le polygone

				// Peindre le polygone avec une couleur transparente
				painter.setBrush(Wt::WBrush(Wt::WColor(config.red, config.green, config.blue, config.opacity)));
				
				Wt::WPen Pen(Wt::PenStyle::SolidLine); // définir un pinceau solide pour dessiner les lignes
				Pen.setColor(Wt::WColor(config.red, config.green, config.blue)); // définir la couleur des lignes
				painter.setPen(Pen); // appliquer le pinceau définit
				painter.drawPath(zoomTransform.map(fillPath)); // dessiner le polygone
				break;
			}
			default : {
				// Initialiser le chemin pour dessiner une ligne simple
				Wt::WPainterPath path
				(
					Wt::WPointF
					(
						Wt::asNumber(model()->data(0, 0)), 
						Wt::asNumber(model()->data(0, config.seriesIds[0]))
					)
				);
				// tracer les points de la ligne
				for (int row = 1; row < model()->rowCount(); ++row) {
					path.lineTo(
						mapToDeviceWithoutTransform(
							Wt::asNumber(model()->data(row, 0)), 
							Wt::asNumber(model()->data(row, config.seriesIds[0]))
						)
					);
				}
				path.closeSubPath(); // fermeture du chemin
				// Dessiner la lignes
				painter.setBrush(Wt::WBrush(Wt::StandardColor::Transparent)); // "supprimer" la couleur de la brosse
				painter.setPen(Wt::WPen(Wt::WColor(config.red, config.green, config.blue, config.opacity)));
				painter.drawPath(zoomTransform.map(path));
				break;
			}
		}
	}
	painter.restore();
	// keep last index in memory
	last_timestamp_ = model()->data(model()->rowCount() - 1 , 0);
}

Wt::WPainterPath Chart::drawCandle(double x, double open, double high, double low, double close, double candleWidth) const
{  
    // Mèche de la bougie
    Wt::WPainterPath path(mapToDeviceWithoutTransform(x, high));
    path.lineTo(mapToDeviceWithoutTransform(x, low));
    // corps de la bougie
    Wt::WPointF topLeft = mapToDeviceWithoutTransform(x - candleWidth/2, std::max(open, close));
    Wt::WPointF bottomRight = mapToDeviceWithoutTransform(x + candleWidth/2, std::min(open, close));
    path.addRect(Wt::WRectF(topLeft, bottomRight));
    
    return path;
}

void Chart::onFileModified() {
	 Wt::log("info") << "Mise à jour du modèle";
    updateModel();
	 Wt::log("info") << "Mise à jour des axes";
    updateAxisRanges();
	 Wt::log("info") << "Mise à jour de la glissière";
	 Wt::log("info") << "Mise à jour du graphique";
    update();
    Wt::log("info") << "update terminé";
}

void Chart::startMonitoring() {
	if (!monitoring_) {
		monitoring_ = true;  // Indique que la surveillance est active
		filereader_->startMonitoring([this]() {
			this->onFileModified();  // Appelle la fonction de mise à jour du graphique
		});
	}
}


void Chart::stopMonitoring() {
	if (monitoring_) {
		monitoring_ = false;
		filereader_->stopMonitoring();
	}
}

void Chart::setupClientMouseHover(Wt::WContainerWidget* displayWidget, const std::vector<HDF5Data::Candle>& data) {
	// Convertir les données du modèle en format JSON
	Wt::Json::Array jsonData;
	for (auto& entry : data) {
		Wt::Json::Object dataPoint;
		dataPoint["timestamp"] = entry.timestamp/1000 ; // timestamp
		dataPoint["open"] = entry.open ; // open price
		dataPoint["high"] = entry.high ; // high price
		dataPoint["low"] = entry.low ; // low price
		dataPoint["close"] = entry.close ; // close price
		dataPoint["volume"] = entry.volume ; // volume
		dataPoint["trades"] = entry.trades ; // number of trades
		jsonData.push_back(dataPoint);
	} 

    // Script JavaScript pour gérer le survol
    std::string js = R"(
        var chartData = )" + Wt::Json::serialize(jsonData) + R"(;
        var chart = )" + this->jsRef() + R"(;
        var display = )" + displayWidget->jsRef() + R"(;

        chart.addEventListener('mousemove', function(event) {
            var config = chart.wtCObj.config;
            if (!config) return;

            var rect = chart.getBoundingClientRect();
            var x = event.clientX - rect.left;
            var y = event.clientY - rect.top;

            // Utiliser la largeur totale du graphique
            var xMin = config.area[0];
            var xMax = config.area[0] + config.area[2];

            // Vérifier si x est dans les limites du graphique
            if (x < xMin || x > xMax) return;

            // Ajuster x pour compenser la marge du graphique
            var adjustedX = x - xMin;

            // Prendre en compte le padding si nécessaire (s'il y en a)
            var paddingX = config.coordinateOverlayPadding[0]; // Padding sur l'axe X
            //adjustedX -= paddingX;  // Ajuster la position X avec le padding

            // Extraire la transformation affine pour l'axe X
            var xTransform = config.xTransforms[0];
            var scaleX = xTransform[0];  // Échelle de l'axe X
            var transX = xTransform[4];  // Translation sur l'axe X

            // Ajuster la position de la souris selon l'échelle et la translation
            var x_data = (adjustedX - transX) / scaleX;

            // Obtenir les informations sur le modèle des données
            var modelStartTimestamp = config.xModelAreas[0][0]; // Timestamp de début
            var modelStepsX = config.xModelAreas[0][2];         // Nombre de pas sur l'axe X

            // Calculer le timestamp correspondant à la position de la souris
            var timestampAtMouse = (modelStartTimestamp + x_data * (modelStepsX / config.area[2]))+((new Date()).getTimezoneOffset()*60);
				
				// Calculer l'index directement à partir du timestamp
				var firstTimestamp = chartData[0].timestamp;
				var lastTimestamp = chartData[chartData.length - 1].timestamp;
				var index = ((firstTimestamp - 30) - timestampAtMouse >= 0) 
				? 0 
				: ((timestampAtMouse - (lastTimestamp + 30) >= 0)
					? chartData.length - 1
					: Math.floor((timestampAtMouse - (firstTimestamp - 30)) / 60));
					
				// S'assurer que l'index est dans les limites valides
				index = Math.max(0, Math.min(index, chartData.length - 1));

				// Récupérer les données de la bougie à l'index calculé
				var data = chartData[index];

            // Extraire les informations de la bougie la plus proche
            var info = 'Timestamp: ' + new Date(data.timestamp * 1000).toLocaleString() + '<br>' +
                       'Open: ' + data.open.toFixed(8) + '<br>' +
                       'High: ' + data.high.toFixed(8) + '<br>' +
                       'Low: ' + data.low.toFixed(8) + '<br>' +
                       'Close: ' + data.close.toFixed(8) + '<br>' +
                       'Volume: ' + data.volume.toFixed(8) + '<br>' +
                       'Trades: ' + data.trades;

            display.innerHTML = info;
        });
    )";

    this->doJavaScript(js);
}


} // namespace Financial
