#include "Vista3D.h"
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QCamera>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <QDebug>

ConfigTurbina3D Turbina3D::configGlobal;

static QQuaternion quaternionDesdeEuler(const QVector3D &anglesGrados)
{
	QQuaternion rotX = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), anglesGrados.x());
	QQuaternion rotY = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), anglesGrados.y());
	QQuaternion rotZ = QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), anglesGrados.z());
	return rotZ * rotY * rotX;
}

// ---------- Turbina3D ----------
Turbina3D::Turbina3D(Qt3DCore::QEntity *parent, int id)
	: Qt3DCore::QEntity(parent), m_id(id), m_anguloActual(0.0)
{
	const ConfigTurbina3D &cfg = Turbina3D::configGlobal;
	const float alturaPoste = cfg.alturaPoste;
	const float baseY = cfg.posicionPoste.y();
	
	m_posicionBase = QVector3D(0,0,0);
	
	// Poste
	auto *torreMesh = new Qt3DExtras::QCylinderMesh();
	torreMesh->setRadius(cfg.radioPoste);
	torreMesh->setLength(alturaPoste);
	auto *torreMaterial = new Qt3DExtras::QPhongMaterial();
	torreMaterial->setDiffuse(QColor(220, 220, 230));
	auto *torreTransform = new Qt3DCore::QTransform();
	torreTransform->setRotation(quaternionDesdeEuler(cfg.rotacionPoste));
	torreTransform->setTranslation(QVector3D(0.0f, baseY + alturaPoste / 2.0f, 0.0f));
	this->addComponent(torreMesh);
	this->addComponent(torreMaterial);
	this->addComponent(torreTransform);
	
	// Góndola
	QVector3D posGondola = cfg.posicionGondola;
	auto *gondolaMesh = new Qt3DExtras::QCylinderMesh();
	gondolaMesh->setRadius(cfg.radioGondola);
	gondolaMesh->setLength(cfg.largoGondola);
	auto *gondolaMaterial = new Qt3DExtras::QPhongMaterial();
	gondolaMaterial->setDiffuse(QColor(240, 240, 250));
	auto *gondolaTransform = new Qt3DCore::QTransform();
	gondolaTransform->setRotation(quaternionDesdeEuler(cfg.rotacionGondola));
	gondolaTransform->setTranslation(QVector3D(posGondola.x(),
											   baseY + alturaPoste + posGondola.y(),
											   posGondola.z()));
	auto *gondolaEntity = new Qt3DCore::QEntity(this);
	gondolaEntity->addComponent(gondolaMesh);
	gondolaEntity->addComponent(gondolaMaterial);
	gondolaEntity->addComponent(gondolaTransform);
	
	// Grupo de aspas
	QVector3D posHelice = cfg.posicionHelice;
	auto *aspasGroup = new Qt3DCore::QEntity(this);
	m_rotacionAspas = new Qt3DCore::QTransform();
	m_rotacionBaseAspas = quaternionDesdeEuler(cfg.rotacionHelice);
	m_rotacionAspas->setRotation(m_rotacionBaseAspas);
	m_rotacionAspas->setTranslation(QVector3D(posHelice.x(),
											  baseY + alturaPoste + posHelice.y(),
											  posHelice.z()));
	aspasGroup->addComponent(m_rotacionAspas);
	
	auto *aspaMaterial = new Qt3DExtras::QPhongMaterial();
	aspaMaterial->setDiffuse(QColor(255, 255, 255));
	
	const float largoAspa = cfg.dimensionAspa.x();
	for (int i = 0; i < 3; ++i) {
		auto *aspaMesh = new Qt3DExtras::QCuboidMesh();
		aspaMesh->setXExtent(largoAspa);
		aspaMesh->setYExtent(cfg.dimensionAspa.y());
		aspaMesh->setZExtent(cfg.dimensionAspa.z());
		
		double angle = i * 120.0;
		QQuaternion rotAspa = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), angle);
		QVector3D offsetLocal(cfg.huecoHelice + largoAspa / 2.0f, 0.0f, 0.0f);
		
		auto *aspaTransform = new Qt3DCore::QTransform();
		aspaTransform->setRotation(rotAspa);
		aspaTransform->setTranslation(rotAspa.rotatedVector(offsetLocal));
		
		auto *aspaEntity = new Qt3DCore::QEntity(aspasGroup);
		aspaEntity->addComponent(aspaMesh);
		aspaEntity->addComponent(aspaMaterial);
		aspaEntity->addComponent(aspaTransform);
	}
}

void Turbina3D::actualizarRotacionAspas(double velocidadViento)
{
	const double factor = 3.0;
	m_anguloActual += velocidadViento * factor;
	QQuaternion rotacionViento = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), m_anguloActual);
	m_rotacionAspas->setRotation(m_rotacionBaseAspas * rotacionViento);
}

// ---------- Vista3D ----------
Vista3D::Vista3D(ParqueEolico *parque, QScreen *screen)
	: Qt3DExtras::Qt3DWindow(screen), m_parque(parque), m_panning(false)
{
	setFlags(Qt::FramelessWindowHint);
	m_rootEntity = new Qt3DCore::QEntity();
	setRootEntity(m_rootEntity);
	crearEscenaBase();
	
	const int numTurbinasMax = 21;
	const float radioOrbita = Turbina3D::configGlobal.radioOrbitaTurbinas;
	for (int i = 0; i < numTurbinasMax; ++i) {
		double angle = i * 360.0 / numTurbinasMax * M_PI / 180.0;
		m_posicionesPrefijadas.append(QVector3D(radioOrbita * cos(angle), 0.0f, radioOrbita * sin(angle)));
	}
	
	connect(m_parque, &ParqueEolico::datosActualizados, this, &Vista3D::actualizarTodasLasTurbinas);
	
	m_animTimer = new QTimer(this);
	connect(m_animTimer, &QTimer::timeout, this, &Vista3D::actualizarAnimacion);
	m_animTimer->start(50);
	
	camera()->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 500.0f);
	camera()->setPosition(QVector3D(30, 18, 30));
	camera()->setViewCenter(QVector3D(0, 3, 0));
	camera()->setUpVector(QVector3D(0, 1, 0));
	defaultFrameGraph()->setClearColor(QColor(135, 206, 235));
	
	this->installEventFilter(this);
	
	crearArboles();
	crearCasa();
	crearCentral();
	
	actualizarTodasLasTurbinas();
}

Vista3D::~Vista3D()
{
	delete m_rootEntity;
}

void Vista3D::actualizarAnimacion()
{
	const auto &turbinasReales = m_parque->getTurbinas();
	for (TurbinaEolica *turbinaReal : turbinasReales) {
		int id = turbinaReal->getId();
		if (m_turbinasVisuales.contains(id)) {
			double velocidad = turbinaReal->getVelocidadViento();
			m_turbinasVisuales[id]->actualizarRotacionAspas(velocidad);
		}
	}
}

bool Vista3D::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == this) {
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::LeftButton) {
				m_panning = true;
				m_lastMousePos = mouseEvent->pos();
				return true;
			}
		} else if (event->type() == QEvent::MouseMove) {
			if (m_panning) {
				QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
				QPoint delta = mouseEvent->pos() - m_lastMousePos;
				float speed = 0.05f;
				QVector3D translation(-delta.x() * speed, 0.0f, -delta.y() * speed);
				camera()->setPosition(camera()->position() + translation);
				camera()->setViewCenter(camera()->viewCenter() + translation);
				m_lastMousePos = mouseEvent->pos();
				return true;
			}
		} else if (event->type() == QEvent::MouseButtonRelease) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::LeftButton) {
				m_panning = false;
				return true;
			}
		} else if (event->type() == QEvent::Wheel) {
			QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
			float delta = wheelEvent->angleDelta().y() / 120.0f;
			float zoomSpeed = 0.8f;
			QVector3D direction = camera()->viewVector().normalized();
			camera()->setPosition(camera()->position() + direction * delta * zoomSpeed);
			return true;
		}
	}
	return Qt3DExtras::Qt3DWindow::eventFilter(obj, event);
}

void Vista3D::crearEscenaBase()
{
	// Suelo
	auto *planeMesh = new Qt3DExtras::QPlaneMesh();
	planeMesh->setWidth(100);
	planeMesh->setHeight(100);
	auto *groundMaterial = new Qt3DExtras::QPhongMaterial();
	groundMaterial->setDiffuse(QColor(34, 139, 34));
	auto *planeTransform = new Qt3DCore::QTransform();
	planeTransform->setTranslation(QVector3D(0, -0.05f, 0));
	auto *groundEntity = new Qt3DCore::QEntity(m_rootEntity);
	groundEntity->addComponent(planeMesh);
	groundEntity->addComponent(groundMaterial);
	groundEntity->addComponent(planeTransform);
	
	// Anillos concéntricos
	const float radii[] = {5.0f, 10.0f, 15.0f, 20.0f};
	for (float r : radii) {
		auto *ringMesh = new Qt3DExtras::QCylinderMesh();
		ringMesh->setRadius(r);
		ringMesh->setLength(0.05f);
		auto *ringMaterial = new Qt3DExtras::QPhongMaterial();
		ringMaterial->setDiffuse(QColor(180, 200, 100));
		auto *ringTransform = new Qt3DCore::QTransform();
		ringTransform->setTranslation(QVector3D(0, -0.1f, 0));
		auto *ringEntity = new Qt3DCore::QEntity(m_rootEntity);
		ringEntity->addComponent(ringMesh);
		ringEntity->addComponent(ringMaterial);
		ringEntity->addComponent(ringTransform);
	}
	
	// Marcas radiales
	for (int i = 0; i < 36; ++i) {
		double angle = i * 10.0 * M_PI / 180.0;
		float radius = 18.0f;
		QVector3D pos(radius * cos(angle), -0.1f, radius * sin(angle));
		auto *markerMesh = new Qt3DExtras::QCylinderMesh();
		markerMesh->setRadius(0.08f);
		markerMesh->setLength(0.1f);
		auto *markerMaterial = new Qt3DExtras::QPhongMaterial();
		markerMaterial->setDiffuse(QColor(200, 220, 150));
		auto *markerTransform = new Qt3DCore::QTransform();
		markerTransform->setTranslation(pos);
		auto *markerEntity = new Qt3DCore::QEntity(m_rootEntity);
		markerEntity->addComponent(markerMesh);
		markerEntity->addComponent(markerMaterial);
		markerEntity->addComponent(markerTransform);
	}
	
	// Iluminación
	auto *ambientLight = new Qt3DCore::QEntity(m_rootEntity);
	auto *ambient = new Qt3DRender::QPointLight(ambientLight);
	ambient->setColor(QColor(100, 100, 110));
	ambient->setIntensity(0.45f);
	ambientLight->addComponent(ambient);
	
	auto *sunLight = new Qt3DCore::QEntity(m_rootEntity);
	auto *dirLight = new Qt3DRender::QDirectionalLight(sunLight);
	dirLight->setColor(QColor(255, 248, 220));
	dirLight->setIntensity(1.2f);
	dirLight->setWorldDirection(QVector3D(-0.6f, -1.0f, -0.4f));
	sunLight->addComponent(dirLight);
	
	auto *fillLight = new Qt3DCore::QEntity(m_rootEntity);
	auto *pointLight = new Qt3DRender::QPointLight(fillLight);
	pointLight->setColor(QColor(210, 180, 140));
	pointLight->setIntensity(0.35f);
	auto *lightTransform = new Qt3DCore::QTransform();
	lightTransform->setTranslation(QVector3D(0, 5, 0));
	fillLight->addComponent(pointLight);
	fillLight->addComponent(lightTransform);
}

void Vista3D::crearArboles()
{
	ConfigArbol cfg;
	const int numArboles = 40;
	QVector<QVector3D> posiciones;
	for (int i = 0; i < numArboles; ++i) {
		float angle = (i * 27.0f) * M_PI / 180.0f;
		float radio = 28.0f + (i % 5) * 1.5f;
		float x = radio * cos(angle);
		float z = radio * sin(angle);
		if (i % 3 == 0) {
			radio = 12.0f + (i % 8);
			x = radio * cos(angle + 0.7f);
			z = radio * sin(angle + 0.7f);
		}
		posiciones.append(QVector3D(x, 0.0f, z));
	}
	
	for (const QVector3D &pos : posiciones) {
		auto *grupoArbol = new Qt3DCore::QEntity(m_rootEntity);
		auto *posTransform = new Qt3DCore::QTransform();
		posTransform->setTranslation(pos);
		grupoArbol->addComponent(posTransform);
		
		// Tronco
		auto *troncoMesh = new Qt3DExtras::QCylinderMesh();
		troncoMesh->setRadius(cfg.radioTronco);
		troncoMesh->setLength(cfg.alturaTronco);
		auto *troncoMaterial = new Qt3DExtras::QPhongMaterial();
		troncoMaterial->setDiffuse(cfg.colorTronco);
		auto *troncoTransform = new Qt3DCore::QTransform();
		troncoTransform->setTranslation(QVector3D(0, cfg.alturaTronco/2, 0));
		auto *troncoEntity = new Qt3DCore::QEntity(grupoArbol);
		troncoEntity->addComponent(troncoMesh);
		troncoEntity->addComponent(troncoMaterial);
		troncoEntity->addComponent(troncoTransform);
		
		// Copa
		auto *copaMesh = new Qt3DExtras::QConeMesh();
		copaMesh->setTopRadius(0.0f);
		copaMesh->setBottomRadius(cfg.radioCopa);
		copaMesh->setLength(cfg.alturaCopa);
		auto *copaMaterial = new Qt3DExtras::QPhongMaterial();
		copaMaterial->setDiffuse(cfg.colorCopa);
		auto *copaTransform = new Qt3DCore::QTransform();
		copaTransform->setTranslation(QVector3D(0, cfg.alturaTronco + cfg.alturaCopa/2, 0));
		auto *copaEntity = new Qt3DCore::QEntity(grupoArbol);
		copaEntity->addComponent(copaMesh);
		copaEntity->addComponent(copaMaterial);
		copaEntity->addComponent(copaTransform);
	}
}

void Vista3D::crearCasa()
{
	ConfigCasa cfg;
	auto *casaGroup = new Qt3DCore::QEntity(m_rootEntity);
	auto *posTransform = new Qt3DCore::QTransform();
	posTransform->setTranslation(cfg.posicion);
	casaGroup->addComponent(posTransform);
	
	// Base
	auto *baseMesh = new Qt3DExtras::QCuboidMesh();
	baseMesh->setXExtent(cfg.dimensionBase.x());
	baseMesh->setYExtent(cfg.dimensionBase.y());
	baseMesh->setZExtent(cfg.dimensionBase.z());
	auto *baseMaterial = new Qt3DExtras::QPhongMaterial();
	baseMaterial->setDiffuse(cfg.colorPared);
	auto *baseTransform = new Qt3DCore::QTransform();
	baseTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y()/2, 0));
	auto *baseEntity = new Qt3DCore::QEntity(casaGroup);
	baseEntity->addComponent(baseMesh);
	baseEntity->addComponent(baseMaterial);
	baseEntity->addComponent(baseTransform);
	
	// Techo inferior
	auto *techoBaseMesh = new Qt3DExtras::QCuboidMesh();
	techoBaseMesh->setXExtent(cfg.dimensionTecho.x());
	techoBaseMesh->setYExtent(cfg.dimensionTecho.y() * 0.5f);
	techoBaseMesh->setZExtent(cfg.dimensionTecho.z());
	auto *techoMaterial = new Qt3DExtras::QPhongMaterial();
	techoMaterial->setDiffuse(cfg.colorTecho);
	auto *techoTransform = new Qt3DCore::QTransform();
	techoTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y() + cfg.dimensionTecho.y()*0.25f, 0));
	auto *techoEntity = new Qt3DCore::QEntity(casaGroup);
	techoEntity->addComponent(techoBaseMesh);
	techoEntity->addComponent(techoMaterial);
	techoEntity->addComponent(techoTransform);
	
	// Techo superior
	auto *techoTopMesh = new Qt3DExtras::QCuboidMesh();
	techoTopMesh->setXExtent(cfg.dimensionTecho.x() * 0.7f);
	techoTopMesh->setYExtent(cfg.dimensionTecho.y() * 0.5f);
	techoTopMesh->setZExtent(cfg.dimensionTecho.z() * 0.7f);
	auto *techoTopTransform = new Qt3DCore::QTransform();
	techoTopTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y() + cfg.dimensionTecho.y()*0.75f, 0));
	auto *techoTopEntity = new Qt3DCore::QEntity(casaGroup);
	techoTopEntity->addComponent(techoTopMesh);
	techoTopEntity->addComponent(techoMaterial);
	techoTopEntity->addComponent(techoTopTransform);
	
	// Puerta
	auto *puertaMesh = new Qt3DExtras::QCuboidMesh();
	puertaMesh->setXExtent(0.8f);
	puertaMesh->setYExtent(1.2f);
	puertaMesh->setZExtent(0.1f);
	auto *puertaMaterial = new Qt3DExtras::QPhongMaterial();
	puertaMaterial->setDiffuse(cfg.colorPuerta);
	auto *puertaTransform = new Qt3DCore::QTransform();
	puertaTransform->setTranslation(QVector3D(0, 0.6f, cfg.dimensionBase.z()/2 + 0.05f));
	auto *puertaEntity = new Qt3DCore::QEntity(casaGroup);
	puertaEntity->addComponent(puertaMesh);
	puertaEntity->addComponent(puertaMaterial);
	puertaEntity->addComponent(puertaTransform);
	
	// Ventanas
	auto *ventanaMaterial = new Qt3DExtras::QPhongMaterial();
	ventanaMaterial->setDiffuse(cfg.colorVentana);
	QVector<QVector3D> ventanasPos = { QVector3D(-1.2f, 1.0f, cfg.dimensionBase.z()/2 + 0.05f),
		QVector3D( 1.2f, 1.0f, cfg.dimensionBase.z()/2 + 0.05f),
		QVector3D(-1.2f, 1.0f, -cfg.dimensionBase.z()/2 - 0.05f),
		QVector3D( 1.2f, 1.0f, -cfg.dimensionBase.z()/2 - 0.05f) };
	for (const QVector3D &pos : ventanasPos) {
		auto *ventanaMesh = new Qt3DExtras::QCuboidMesh();
		ventanaMesh->setXExtent(0.8f);
		ventanaMesh->setYExtent(0.8f);
		ventanaMesh->setZExtent(0.05f);
		auto *ventanaTransform = new Qt3DCore::QTransform();
		ventanaTransform->setTranslation(pos);
		auto *ventanaEntity = new Qt3DCore::QEntity(casaGroup);
		ventanaEntity->addComponent(ventanaMesh);
		ventanaEntity->addComponent(ventanaMaterial);
		ventanaEntity->addComponent(ventanaTransform);
	}
}

void Vista3D::crearCentral()
{
	ConfigCentral cfg;
	auto *centralGroup = new Qt3DCore::QEntity(m_rootEntity);
	auto *posTransform = new Qt3DCore::QTransform();
	posTransform->setTranslation(cfg.posicion);
	centralGroup->addComponent(posTransform);
	
	// Base
	auto *baseMesh = new Qt3DExtras::QCuboidMesh();
	baseMesh->setXExtent(cfg.dimensionBase.x());
	baseMesh->setYExtent(cfg.dimensionBase.y());
	baseMesh->setZExtent(cfg.dimensionBase.z());
	auto *baseMaterial = new Qt3DExtras::QPhongMaterial();
	baseMaterial->setDiffuse(cfg.colorPared);
	auto *baseTransform = new Qt3DCore::QTransform();
	baseTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y()/2, 0));
	auto *baseEntity = new Qt3DCore::QEntity(centralGroup);
	baseEntity->addComponent(baseMesh);
	baseEntity->addComponent(baseMaterial);
	baseEntity->addComponent(baseTransform);
	
	// Techo
	auto *techoMesh = new Qt3DExtras::QCuboidMesh();
	techoMesh->setXExtent(cfg.dimensionTecho.x());
	techoMesh->setYExtent(cfg.dimensionTecho.y());
	techoMesh->setZExtent(cfg.dimensionTecho.z());
	auto *techoMaterial = new Qt3DExtras::QPhongMaterial();
	techoMaterial->setDiffuse(cfg.colorTecho);
	auto *techoTransform = new Qt3DCore::QTransform();
	techoTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y() + cfg.dimensionTecho.y()/2, 0));
	auto *techoEntity = new Qt3DCore::QEntity(centralGroup);
	techoEntity->addComponent(techoMesh);
	techoEntity->addComponent(techoMaterial);
	techoEntity->addComponent(techoTransform);
	
	// Antena
	auto *antenaMesh = new Qt3DExtras::QCylinderMesh();
	antenaMesh->setRadius(cfg.radioAntena);
	antenaMesh->setLength(cfg.alturaAntena);
	auto *antenaMaterial = new Qt3DExtras::QPhongMaterial();
	antenaMaterial->setDiffuse(QColor(192,192,192));
	auto *antenaTransform = new Qt3DCore::QTransform();
	antenaTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y() + cfg.dimensionTecho.y() + cfg.alturaAntena/2, 0));
	auto *antenaEntity = new Qt3DCore::QEntity(centralGroup);
	antenaEntity->addComponent(antenaMesh);
	antenaEntity->addComponent(antenaMaterial);
	antenaEntity->addComponent(antenaTransform);
	
	// Esfera en la punta (QSphereMesh ahora está incluido)
	auto *esferaMesh = new Qt3DExtras::QSphereMesh();
	esferaMesh->setRadius(0.2f);
	auto *esferaMaterial = new Qt3DExtras::QPhongMaterial();
	esferaMaterial->setDiffuse(QColor(255,215,0));
	auto *esferaTransform = new Qt3DCore::QTransform();
	esferaTransform->setTranslation(QVector3D(0, cfg.dimensionBase.y() + cfg.dimensionTecho.y() + cfg.alturaAntena, 0));
	auto *esferaEntity = new Qt3DCore::QEntity(centralGroup);
	esferaEntity->addComponent(esferaMesh);
	esferaEntity->addComponent(esferaMaterial);
	esferaEntity->addComponent(esferaTransform);
}

Qt3DCore::QEntity* Vista3D::crearCilindroEntrePuntos(const QVector3D &p1, const QVector3D &p2,
													 float radio, const QColor &color,
													 Qt3DCore::QEntity *parent)
{
	QVector3D direccion = p2 - p1;
	float longitud = direccion.length();
	if (longitud < 0.01f) return nullptr;
	
	QVector3D centro = (p1 + p2) / 2.0f;
	QVector3D ejeY(0,1,0);
	QQuaternion rotacion = QQuaternion::rotationTo(ejeY, direccion.normalized());
	
	auto *cableEntity = new Qt3DCore::QEntity(parent);
	auto *mesh = new Qt3DExtras::QCylinderMesh();
	mesh->setRadius(radio);
	mesh->setLength(longitud);
	auto *material = new Qt3DExtras::QPhongMaterial();
	material->setDiffuse(color);
	auto *transform = new Qt3DCore::QTransform();
	transform->setTranslation(centro);
	transform->setRotation(rotacion);
	
	cableEntity->addComponent(mesh);
	cableEntity->addComponent(material);
	cableEntity->addComponent(transform);
	return cableEntity;
}

void Vista3D::crearCables()
{
	for (auto *cable : m_cables) {
		delete cable;
	}
	m_cables.clear();
	
	ConfigCable cfgCable;
	ConfigCentral cfgCentral;
	QVector3D centralPos = cfgCentral.posicion;
	float alturaConexionCentral = cfgCentral.dimensionBase.y() + cfgCentral.dimensionTecho.y();
	QVector3D puntoCentral(centralPos.x(), centralPos.y() + alturaConexionCentral, centralPos.z());
	
	for (auto it = m_turbinasVisuales.begin(); it != m_turbinasVisuales.end(); ++it) {
		Turbina3D *turbina = it.value();
		QVector3D baseTurbina = turbina->getPosicionBase();
		const ConfigTurbina3D &cfgTurb = Turbina3D::configGlobal;
		float alturaConexion = cfgTurb.alturaPoste + cfgTurb.posicionGondola.y();
		QVector3D puntoTurbina(baseTurbina.x(), baseTurbina.y() + alturaConexion, baseTurbina.z());
		
		Qt3DCore::QEntity *cable = crearCilindroEntrePuntos(puntoTurbina, puntoCentral,
															cfgCable.radio, cfgCable.color,
															m_rootEntity);
		if (cable)
			m_cables.append(cable);
	}
}

void Vista3D::agregarTurbinaVisual(int id, const QVector3D &posicionBaseMundial)
{
	if (m_contenedoresTurbinas.contains(id)) return;
	
	auto *contenedor = new Qt3DCore::QEntity(m_rootEntity);
	auto *posTransform = new Qt3DCore::QTransform();
	posTransform->setTranslation(posicionBaseMundial);
	contenedor->addComponent(posTransform);
	
	Turbina3D *turbina = new Turbina3D(contenedor, id);
	turbina->setPosicionBase(posicionBaseMundial);   // <-- ahora funciona
	
	m_contenedoresTurbinas[id] = contenedor;
	m_turbinasVisuales[id] = turbina;
}

void Vista3D::eliminarTurbinaVisual(int id)
{
	if (auto contenedor = m_contenedoresTurbinas.take(id)) {
		delete contenedor;
		m_turbinasVisuales.remove(id);
	}
}

void Vista3D::actualizarTodasLasTurbinas()
{
	const auto &turbinasReales = m_parque->getTurbinas();
	const int maxTurbinas = 21;
	int count = 0;
	QSet<int> idsReales;
	
	for (auto *t : turbinasReales) {
		if (count >= maxTurbinas) break;
		int id = t->getId();
		idsReales.insert(id);
		if (!m_contenedoresTurbinas.contains(id)) {
			int idx = m_contenedoresTurbinas.size();
			if (idx < m_posicionesPrefijadas.size()) {
				agregarTurbinaVisual(id, m_posicionesPrefijadas[idx]);
			} else {
				agregarTurbinaVisual(id, QVector3D(idx * 2.5f, 0.0f, 0.0f));
			}
		}
		++count;
	}
	
	QList<int> idsToRemove;
	for (int id : m_contenedoresTurbinas.keys()) {
		if (!idsReales.contains(id)) {
			idsToRemove.append(id);
		}
	}
	for (int id : idsToRemove) {
		eliminarTurbinaVisual(id);
	}
	
	crearCables();
}
