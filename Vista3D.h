#ifndef VISTA3D_H
#define VISTA3D_H

#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DExtras/QPlaneMesh>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QVector3D>
#include <QQuaternion>
#include <QTimer>
#include "ParqueEolico.h"

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
		
		static ConfigTurbina3D configGlobal;
		
private:
	int m_id;
	Qt3DCore::QTransform *m_rotacionAspas;
	QQuaternion m_rotacionBaseAspas;
	double m_anguloActual = 0.0;
};

class Vista3D : public Qt3DExtras::Qt3DWindow
{
	Q_OBJECT
public:
		explicit Vista3D(ParqueEolico *parque, QScreen *screen = nullptr);
		~Vista3D();
		
		public slots:
			void actualizarTodasLasTurbinas();   // Sincroniza creación/eliminación de turbinas
			
			private slots:
				void actualizarAnimacion();           // Animación continua de las hélices
				
private:
	void crearEscenaBase();
	void agregarTurbinaVisual(int id, const QVector3D &posicionBaseMundial);
	void eliminarTurbinaVisual(int id);
	
	bool eventFilter(QObject *obj, QEvent *event) override;
	
	ParqueEolico *m_parque;
	Qt3DCore::QEntity *m_rootEntity;
	QMap<int, Qt3DCore::QEntity*> m_contenedoresTurbinas;
	QMap<int, Turbina3D*> m_turbinasVisuales;
	QVector<QVector3D> m_posicionesPrefijadas;  // 21 posiciones
	QPoint m_lastMousePos;
	bool m_panning;
	QTimer *m_animTimer;   // Temporizador para animación continua
};

#endif // VISTA3D_H
