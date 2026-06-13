#ifndef VISTA3D_H
#define VISTA3D_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>   // <-- necesario para la antena
#include <QVector>
#include <QMap>
#include <QSet>
#include <QVector3D>
#include <QQuaternion>
#include <QTimer>
#include "ParqueEolico.h"

// Estructuras de configuración para nuevos elementos
struct ConfigArbol
{
	float alturaTronco = 1.2f;
	float radioTronco = 0.3f;
	float alturaCopa = 1.0f;
	float radioCopa = 0.8f;
	QColor colorTronco = QColor(139, 69, 19);
	QColor colorCopa = QColor(34, 139, 34);
};

struct ConfigCasa
{
	QVector3D posicion = QVector3D(12.0f, 0.0f, 12.0f);
	QVector3D dimensionBase = QVector3D(3.0f, 2.5f, 3.0f);
	QVector3D dimensionTecho = QVector3D(3.5f, 1.2f, 3.5f);
	QColor colorPared = QColor(210, 180, 140);
	QColor colorTecho = QColor(160, 82, 45);
	QColor colorPuerta = QColor(101, 67, 33);
	QColor colorVentana = QColor(135, 206, 235);
};

struct ConfigCentral
{
	QVector3D posicion = QVector3D(-15.0f, 0.0f, -12.0f);
	QVector3D dimensionBase = QVector3D(5.0f, 3.5f, 5.0f);
	QVector3D dimensionTecho = QVector3D(5.5f, 1.5f, 5.5f);
	float alturaAntena = 2.0f;
	float radioAntena = 0.1f;
	QColor colorPared = QColor(169, 169, 169);
	QColor colorTecho = QColor(128, 128, 128);
};

struct ConfigCable
{
	float radio = 0.05f;
	QColor color = QColor(100, 100, 100);
};

struct ConfigTurbina3D
{
	float radioOrbitaTurbinas = 24.0f;
	
	QVector3D posicionPoste = QVector3D(0.0f, 0.0f, 0.0f);
	QVector3D rotacionPoste = QVector3D(0.0f, 0.0f, 0.0f);
	float radioPoste  = 0.5f;
	float alturaPoste = 19.5f;
	
	QVector3D posicionGondola = QVector3D(0.0f, -9.4f, 0.0f);
	QVector3D rotacionGondola = QVector3D(90.0f, 0.0f, 90.0f);
	float radioGondola = 0.4f;
	float largoGondola = 3.8f;
	
	QVector3D posicionHelice = QVector3D(0.0f, -9.4f, 2.0f);
	QVector3D rotacionHelice = QVector3D(90.0f, 0.0f, 0.0f);
	QVector3D dimensionAspa = QVector3D(6.2f, 0.26f, 0.50f);
	float huecoHelice = 0.01f;
};

class Turbina3D : public Qt3DCore::QEntity
{
	Q_OBJECT
public:
		Turbina3D(Qt3DCore::QEntity *parent, int id);
		void actualizarRotacionAspas(double velocidadViento);
		int getId() const { return m_id; }
		QVector3D getPosicionBase() const { return m_posicionBase; }
		void setPosicionBase(const QVector3D &pos) { m_posicionBase = pos; }   // <-- AÑADIDO
		
		static ConfigTurbina3D configGlobal;
		
private:
	int m_id;
	Qt3DCore::QTransform *m_rotacionAspas;
	QQuaternion m_rotacionBaseAspas;
	double m_anguloActual = 0.0;
	QVector3D m_posicionBase;
};

class Vista3D : public Qt3DExtras::Qt3DWindow
{
	Q_OBJECT
public:
		explicit Vista3D(ParqueEolico *parque, QScreen *screen = nullptr);
		~Vista3D();
		
		public slots:
			void actualizarTodasLasTurbinas();
			
			private slots:
				void actualizarAnimacion();
				
private:
	void crearEscenaBase();
	void agregarTurbinaVisual(int id, const QVector3D &posicionBaseMundial);
	void eliminarTurbinaVisual(int id);
	
	void crearArboles();
	void crearCasa();
	void crearCentral();
	void crearCables();
	
	Qt3DCore::QEntity* crearCilindroEntrePuntos(const QVector3D &p1, const QVector3D &p2,
												float radio, const QColor &color,
												Qt3DCore::QEntity *parent);
	
	bool eventFilter(QObject *obj, QEvent *event) override;
	
	ParqueEolico *m_parque;
	Qt3DCore::QEntity *m_rootEntity;
	QMap<int, Qt3DCore::QEntity*> m_contenedoresTurbinas;
	QMap<int, Turbina3D*> m_turbinasVisuales;
	QVector<QVector3D> m_posicionesPrefijadas;
	QPoint m_lastMousePos;
	bool m_panning;
	QTimer *m_animTimer;
	
	QVector<Qt3DCore::QEntity*> m_cables;
};

#endif // VISTA3D_H
